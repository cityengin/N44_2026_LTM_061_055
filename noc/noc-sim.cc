#include "noc-sim.h"

#include "noc-header.h"
#include "noc-router.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <memory>
#include <unordered_map>

namespace noc
{

using namespace ns3;

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

} // namespace noc
