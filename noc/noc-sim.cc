#include "noc-sim.h"

#include "format.h"
#include "noc-header.h"
#include "noc-router.h"
#include "phi.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <limits>
#include <memory>
#include <unordered_map>

namespace noc
{

using namespace ns3;

namespace
{

// Số chu kỳ tới lần tạo gói kế tiếp (≥ 1): khoảng cách hình học của Bernoulli(p) mỗi chu kỳ.
int64_t
GeometricGap(double p, const Ptr<UniformRandomVariable>& rv)
{
    if (p >= 1.0)
    {
        return 1;
    }
    const double u = 1.0 - rv->GetValue(); // (0, 1]
    const auto g = static_cast<int64_t>(std::ceil(std::log(u) / std::log1p(-p)));
    return std::max<int64_t>(g, 1);
}

} // namespace

SimResult
RunSim(const SimConfig& cfg)
{
    const Mesh mesh(cfg.w, cfg.h);
    const int n = mesh.NumNodes();
    const int64_t winStart = cfg.warmup;
    const int64_t winEnd = cfg.warmup + cfg.measure;

    SimResult res;
    res.seed = RngSeedManager::GetSeed();
    res.run = RngSeedManager::GetRun();
    res.chanTx.assign(mesh.NumChannels(), 0);

    const std::vector<Flow> flows = MakeFlows(cfg.pattern, mesh);
    const ChannelPressure cp = ComputePressure(cfg.algo, mesh, flows);
    const double norm = NormFactor(cfg.pattern, mesh);
    for (int ch = 0; ch < mesh.NumChannels(); ++ch)
    {
        res.rhoHop.push_back(cp.hop[ch] * norm);
        res.rhoPath.push_back(cp.path[ch] * norm);
    }
    res.phiHop = MaxPressure(res.rhoHop).phi;

    NodeContainer nodes;
    nodes.Create(n);
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("16Gbps"));
    p2p.SetDeviceAttribute("InterframeGap", TimeValue(Seconds(0)));
    p2p.SetChannelAttribute("Delay", TimeValue(Seconds(0)));
    p2p.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", StringValue("100000p"));

    // devs[id][dir]: device trên node id nối tới hàng xóm theo hướng dir (id logic = y·W + x).
    std::vector<std::array<Ptr<NetDevice>, kNumLinkDirs>> devs(n);
    for (int u = 0; u < n; ++u)
    {
        for (Dir d : {Dir::E, Dir::S}) // mỗi link PPP hai chiều chỉ cài một lần
        {
            const int v = mesh.Neighbor(u, d);
            if (v < 0)
            {
                continue;
            }
            NetDeviceContainer pair = p2p.Install(nodes.Get(u), nodes.Get(v));
            devs[u][static_cast<int>(d)] = pair.Get(0);
            devs[v][static_cast<int>(Opposite(d))] = pair.Get(1);
        }
    }

    std::unordered_map<uint32_t, int64_t> pending; // gói của cửa sổ chưa tới: seq → lúc tạo
    std::vector<int64_t> latencies;
    long hopsSum = 0;
    uint32_t nextSeq = 0;

    auto deliver = [&](const NoCHeader& hdr, int64_t createdNs) {
        const int64_t now = Simulator::Now().GetNanoSeconds();
        if (hdr.GetHops() != mesh.Distance(hdr.GetSrc(), hdr.GetDst()))
        {
            ++res.hopMismatch;
        }
        if (now >= winStart && now < winEnd)
        {
            ++res.arrivedInWindow;
        }
        auto it = pending.find(hdr.GetSeq());
        if (it != pending.end())
        {
            latencies.push_back(now - createdNs);
            hopsSum += hdr.GetHops();
            pending.erase(it);
            if (pending.empty() && now >= winEnd)
            {
                Simulator::Stop();
            }
        }
    };

    std::vector<std::unique_ptr<NocRouter>> routers;
    for (int id = 0; id < n; ++id)
    {
        routers.push_back(std::make_unique<NocRouter>(id, mesh, cfg.algo, deliver));
        for (Dir d : kDirOrder)
        {
            if (devs[id][static_cast<int>(d)])
            {
                routers[id]->AttachDevice(d, devs[id][static_cast<int>(d)]);
            }
        }
        routers[id]->AssignStreams(3 * id);
    }

    Callback<void, Ptr<const Packet>> onDrop([&res](Ptr<const Packet>) { ++res.drops; });
    for (int ch = 0; ch < mesh.NumChannels(); ++ch)
    {
        const Channel& c = mesh.GetChannel(ch);
        Ptr<PointToPointNetDevice> dev =
            DynamicCast<PointToPointNetDevice>(devs[c.u][static_cast<int>(c.dir)]);
        dev->TraceConnectWithoutContext(
            "PhyTxBegin",
            Callback<void, Ptr<const Packet>>([&res, ch, winStart, winEnd](Ptr<const Packet> p) {
                const uint32_t size = p->GetSize();
                NS_ABORT_MSG_UNLESS(size == kFrameBytes,
                                    "frame trên dây " << size << " B, phải là " << kFrameBytes);
                res.minFrame = res.minFrame == 0 ? size : std::min(res.minFrame, size);
                res.maxFrame = std::max(res.maxFrame, size);
                const int64_t now = Simulator::Now().GetNanoSeconds();
                if (now >= winStart && now < winEnd)
                {
                    ++res.chanTx[ch];
                }
                if (res.txStartNs.size() < 64)
                {
                    res.txStartNs.push_back(now);
                }
            }));
        dev->GetQueue()->TraceConnectWithoutContext("Drop", onDrop);
        dev->TraceConnectWithoutContext("PhyRxDrop", onDrop);
    }

    // Tạo một gói tại src lúc này; gói tạo trong cửa sổ đo được đưa vào pending.
    auto create = [&](int src, int dst) {
        const int64_t now = Simulator::Now().GetNanoSeconds();
        const uint32_t seq = nextSeq++;
        if (now >= winStart && now < winEnd)
        {
            pending.emplace(seq, now);
            ++res.generated;
        }
        routers[src]->Inject(dst, seq);
    };
    for (const Injection& inj : cfg.injections)
    {
        Simulator::Schedule(NanoSeconds(inj.timeNs), [&create, inj]() { create(inj.src, inj.dst); });
    }

    // Bộ tạo traffic: mỗi chu kỳ mỗi node phát với xác suất PIR (Bernoulli), cài bằng khoảng cách
    // hình học. Node tự map vào chính nó không phát. Vẫn phát trong pha drain.
    std::vector<int> fixedDst(n, -1); // transpose: đích cố định
    std::vector<char> sends(n, 0);
    for (const Flow& f : flows)
    {
        sends[f.src] = 1;
        fixedDst[f.src] = f.dst;
    }
    std::vector<Ptr<UniformRandomVariable>> gapRv(n);
    std::vector<Ptr<UniformRandomVariable>> dstRv(n);
    for (int id = 0; id < n; ++id)
    {
        gapRv[id] = CreateObject<UniformRandomVariable>();
        gapRv[id]->SetStream(3 * id + 1);
        dstRv[id] = CreateObject<UniformRandomVariable>();
        dstRv[id]->SetStream(3 * id + 2);
    }
    std::function<void(int)> generate = [&](int src) {
        int dst = fixedDst[src];
        if (cfg.pattern == Pattern::Uniform)
        {
            const int r = static_cast<int>(dstRv[src]->GetInteger(0, n - 2));
            dst = r >= src ? r + 1 : r;
        }
        create(src, dst);
        Simulator::Schedule(NanoSeconds(GeometricGap(cfg.pir, gapRv[src])), [&generate, src]() {
            generate(src);
        });
    };
    if (cfg.injections.empty() && cfg.pir > 0.0)
    {
        for (int id = 0; id < n; ++id)
        {
            if (sends[id])
            {
                // Lần thử Bernoulli đầu tiên ở chu kỳ 0, nên lần phát đầu ở chu kỳ G − 1.
                const int64_t first = GeometricGap(cfg.pir, gapRv[id]) - 1;
                Simulator::Schedule(NanoSeconds(first), [&generate, id]() { generate(id); });
            }
        }
    }

    Simulator::Schedule(NanoSeconds(winEnd), [&pending]() {
        if (pending.empty())
        {
            Simulator::Stop();
        }
    });
    Simulator::Stop(NanoSeconds(winEnd + cfg.drain));
    Simulator::Run();
    res.stopNs = Simulator::Now().GetNanoSeconds();

    res.delivered = static_cast<long>(latencies.size());
    res.undelivered = res.generated - res.delivered;
    res.saturated = res.undelivered > 0;
    const double nan = std::numeric_limits<double>::quiet_NaN();
    double latSum = 0.0;
    for (int64_t l : latencies)
    {
        latSum += static_cast<double>(l);
    }
    res.latAvg = res.delivered > 0 ? latSum / res.delivered : nan;
    res.hopsAvg = res.delivered > 0 ? static_cast<double>(hopsSum) / res.delivered : nan;
    double lbSum = latSum;
    for (const auto& [seq, created] : pending)
    {
        lbSum += static_cast<double>(res.stopNs - created);
    }
    res.latAvgLb = res.generated > 0 ? lbSum / res.generated : nan;
    if (!latencies.empty())
    {
        std::sort(latencies.begin(), latencies.end());
        const auto rank = static_cast<std::size_t>(std::ceil(0.95 * latencies.size()));
        res.latP95 = static_cast<double>(latencies[std::max<std::size_t>(rank, 1) - 1]);
    }
    else
    {
        res.latP95 = nan;
    }
    const double denom = static_cast<double>(n) * static_cast<double>(cfg.measure);
    res.throughput = kPacketFlits * static_cast<double>(res.arrivedInWindow) / denom;
    res.offered = kPacketFlits * static_cast<double>(res.generated) / denom;

    Simulator::Destroy();
    return res;
}

namespace
{

std::string
SimKey(const SimConfig& cfg, const SimResult& r)
{
    const std::string variant = VariantName(cfg.algo);
    return std::string("routing=") + AlgoName(cfg.algo) +
           (variant.empty() ? "" : ",variant=" + variant) + ",traffic=" + PatternName(cfg.pattern) +
           ",mesh=" + std::to_string(cfg.w) + "x" + std::to_string(cfg.h) +
           ",pir=" + Short(cfg.pir) + ",seed=" + std::to_string(r.seed) +
           ",run=" + std::to_string(r.run);
}

} // namespace

std::string
FormatResult(const SimConfig& cfg, const SimResult& r)
{
    const double pirMax = PirMax(r.phiHop);
    return "RESULT,mode=sim," + SimKey(cfg, r) + ",warmup=" + std::to_string(cfg.warmup) +
           ",measure=" + std::to_string(cfg.measure) + ",drain=" + std::to_string(cfg.drain) +
           ",generated=" + std::to_string(r.generated) + ",delivered=" +
           std::to_string(r.delivered) + ",undelivered=" + std::to_string(r.undelivered) +
           ",saturated=" + (r.saturated ? "1" : "0") + ",drops=" + std::to_string(r.drops) +
           ",hop_mismatch=" + std::to_string(r.hopMismatch) + ",lat_avg=" + Fixed(r.latAvg) +
           ",lat_avg_lb=" + Fixed(r.latAvgLb) + ",lat_p95=" + Fixed(r.latP95) +
           ",hops_avg=" + Fixed(r.hopsAvg) + ",throughput=" + Fixed(r.throughput) +
           ",offered=" + Fixed(r.offered) + ",phi_hop=" + Fixed(r.phiHop) +
           ",pir_max_hop=" + Fixed(pirMax) + ",pir_ratio=" + Fixed(cfg.pir / pirMax) +
           ",stop=" + std::to_string(r.stopNs);
}

void
RunSimMode(const SimConfig& cfg, std::ostream& os)
{
    const SimResult r = RunSim(cfg);
    os << FormatResult(cfg, r) << "\n";
    if (!cfg.channels)
    {
        return;
    }
    const Mesh mesh(cfg.w, cfg.h);
    const std::string key = SimKey(cfg, r);
    const double perPacket = kCyclesPerPacket;
    for (int ch = 0; ch < mesh.NumChannels(); ++ch)
    {
        os << "SIMCHAN," << key << ",ch=" << mesh.ChannelName(ch) << ",tx=" << r.chanTx[ch]
           << ",u_meas=" << Fixed(r.chanTx[ch] * perPacket / cfg.measure)
           << ",u_hop=" << Fixed(r.rhoHop[ch] * cfg.pir * perPacket)
           << ",u_path=" << Fixed(r.rhoPath[ch] * cfg.pir * perPacket) << "\n";
    }
}

} // namespace noc
