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
    res.hopsHist.assign(cfg.w + cfg.h - 1, 0);

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
            const int64_t lat = now - createdNs;
            latencies.push_back(lat);
            hopsSum += hdr.GetHops();
            if (hdr.GetHops() < res.hopsHist.size())
            {
                ++res.hopsHist[hdr.GetHops()];
            }
            const auto slack = static_cast<double>(lat - kCyclesPerPacket * hdr.GetHops());
            res.slackMin = std::isnan(res.slackMin) ? slack : std::min(res.slackMin, slack);
            res.slackMax = std::isnan(res.slackMax) ? slack : std::max(res.slackMax, slack);
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
    res.latP95 = PercentileNearestRank(std::move(latencies), 0.95);
    const double denom = static_cast<double>(n) * static_cast<double>(cfg.measure);
    res.throughput = kPacketFlits * static_cast<double>(res.arrivedInWindow) / denom;
    res.offered = kPacketFlits * static_cast<double>(res.generated) / denom;

    Simulator::Destroy();
    return res;
}

double
PercentileNearestRank(std::vector<int64_t> v, double q)
{
    if (v.empty())
    {
        return std::numeric_limits<double>::quiet_NaN();
    }
    std::sort(v.begin(), v.end());
    // Trừ 1e-9 để q·n đúng bằng số nguyên (vd 0.95·100) không bị làm tròn lên hạng kế tiếp.
    const auto rank = static_cast<std::size_t>(std::ceil(q * v.size() - 1e-9));
    return static_cast<double>(v[std::clamp<std::size_t>(rank, 1, v.size()) - 1]);
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

namespace
{

// Khôi phục RngRun ban đầu khi ra khỏi phạm vi.
class RngRunGuard
{
  public:
    RngRunGuard()
        : m_run(RngSeedManager::GetRun())
    {
    }

    ~RngRunGuard() { RngSeedManager::SetRun(m_run); }

    RngRunGuard(const RngRunGuard&) = delete;
    RngRunGuard& operator=(const RngRunGuard&) = delete;

  private:
    uint64_t m_run;
};

// t_{0.975, df}: phân vị hai phía α = 0.05.
double
TCrit975(int df)
{
    static const double table[] =
        {12.706, 4.303, 3.182, 2.776, 2.571, 2.447, 2.365, 2.306, 2.262, 2.228};
    return df >= 1 && df <= 10 ? table[df - 1] : 1.96;
}

struct Sample
{
    double mean;
    double sd; // độ lệch chuẩn mẫu (chia n − 1)
};

Sample
Describe(const std::vector<double>& v)
{
    double sum = 0.0;
    for (double x : v)
    {
        sum += x;
    }
    const double mean = sum / v.size();
    double ss = 0.0;
    for (double x : v)
    {
        ss += (x - mean) * (x - mean);
    }
    return {mean, v.size() > 1 ? std::sqrt(ss / (v.size() - 1)) : 0.0};
}

// Tải thấp (XY, PIR 0.001): bốn kiểm tra tách riêng mô hình và bộ tạo traffic.
// Trả về số kiểm tra FAIL.
int
CheckLowLoad(Pattern pattern, std::ostream& os)
{
    SimConfig cfg;
    cfg.algo = Algo::XY;
    cfg.pattern = pattern;
    cfg.pir = 0.001;
    cfg.warmup = 1000;
    cfg.measure = 100000;
    cfg.drain = 20000;
    const SimResult r = RunSim(cfg);
    os << FormatResult(cfg, r) << "\n";

    // Phân phối số hop lý thuyết: mỗi nguồn phát cùng tốc độ nên mỗi luồng có xác suất 1/flows.
    const Mesh mesh(cfg.w, cfg.h);
    const std::vector<Flow> flows = MakeFlows(pattern, mesh);
    double sumD = 0.0;
    double sumD2 = 0.0;
    int maxD = 0;
    for (const Flow& f : flows)
    {
        const int d = mesh.Distance(f.src, f.dst);
        sumD += d;
        sumD2 += static_cast<double>(d) * d;
        maxD = std::max(maxD, d);
    }
    const double hopsMean = sumD / flows.size();                     // 224/42
    const double hopsVar = sumD2 / flows.size() - hopsMean * hopsMean; // 1568/42 − (224/42)²

    const std::string key = std::string("routing=XY,traffic=") + PatternName(pattern) +
                            ",pir=" + Short(cfg.pir) + ",run=" + std::to_string(r.run);
    int failures = 0;
    auto report = [&](const char* check, const char* kind, bool ok, const std::string& detail) {
        os << "RESULT,mode=simcheck,check=" << check << ",kind=" << kind << "," << key << ","
           << detail << ",status=" << (ok ? "PASS" : "FAIL") << "\n";
        failures += ok ? 0 : 1;
    };

    // a. HARD, từng gói: lat_i ≥ 16·hops_i (store-and-forward không thể nhanh hơn).
    report("low_load_lat_ge_16hops",
           "hard",
           r.delivered > 0 && r.slackMin >= 0.0,
           "delivered=" + std::to_string(r.delivered) + ",slack_min=" + Short(r.slackMin) +
               ",slack_max=" + Short(r.slackMax));

    // b. Mô hình: thời gian chờ queue trung bình ở tải thấp nằm trong [0, 1] chu kỳ.
    const double queueDelay = r.latAvg - kCyclesPerPacket * r.hopsAvg;
    report("low_load_queue_delay",
           "model",
           queueDelay >= 0.0 && queueDelay <= 1.0,
           "lat_avg=" + Fixed(r.latAvg) + ",hops_avg=" + Fixed(r.hopsAvg) +
               ",queue_delay=" + Fixed(queueDelay) + ",min=0,max=1");

    // c. Bộ tạo traffic: |hops_avg − μ| ≤ 4·SE, SE = √(σ²_lý_thuyết / delivered).
    // Kèm số gói có số hop lớn nhất để giải thích lat_p95.
    const double se = std::sqrt(hopsVar / r.delivered);
    const double z = (r.hopsAvg - hopsMean) / se;
    const long nMax = r.hopsHist[maxD];
    const auto rank = static_cast<long>(std::ceil(0.95 * r.delivered - 1e-9));
    report("low_load_hops",
           "stat",
           std::abs(z) <= 4.0,
           "hops_avg=" + Fixed(r.hopsAvg) + ",hops_expect=" + Fixed(hopsMean) +
               ",var_theory=" + Fixed(hopsVar) + ",se=" + Fixed(se) + ",z=" + Fixed(z, 3) +
               ",z_max=4,hmax=" + std::to_string(maxD) + ",n_hmax=" + std::to_string(nMax) +
               ",frac_hmax=" + Fixed(static_cast<double>(nMax) / r.delivered) +
               ",n_hmax_for_p95_eq_hmax=" + std::to_string(r.delivered - rank + 1) +
               ",lat_p95=" + Short(r.latP95));

    // d. Throughput và drop (giữ từ check cũ).
    const double thrExpect = cfg.pir * kPacketFlits * flows.size() / mesh.NumNodes();
    const double thrErr = std::abs(r.throughput - thrExpect) / thrExpect;
    report("low_load_throughput",
           "stat",
           r.drops == 0 && r.undelivered == 0 && thrErr <= 0.05,
           "throughput=" + Fixed(r.throughput) + ",thr_expect=" + Fixed(thrExpect) +
               ",thr_err=" + Fixed(thrErr) + ",thr_tol=0.05,drops=" + std::to_string(r.drops) +
               ",undelivered=" + std::to_string(r.undelivered));
    return failures;
}

// NF/t1 và NF_WS/t2 là ảnh gương qua R nên phải cho cùng phân phối kết quả.
// So mean của lat_avg và throughput qua RngRun 1..runs: đạt nếu |Δ| ≤ t·√(s₁²/n + s₂²/n).
bool
CheckMirror(int runs, std::ostream& os)
{
    std::vector<double> latA;
    std::vector<double> latB;
    std::vector<double> thrA;
    std::vector<double> thrB;
    for (int run = 1; run <= runs; ++run)
    {
        RngSeedManager::SetRun(run);
        for (auto [algo, pattern] : {std::pair{Algo::NF, Pattern::Transpose1},
                                     std::pair{Algo::NF_WS, Pattern::Transpose2}})
        {
            SimConfig cfg;
            cfg.algo = algo;
            cfg.pattern = pattern;
            cfg.pir = 0.02;
            const SimResult r = RunSim(cfg);
            os << FormatResult(cfg, r) << "\n";
            (algo == Algo::NF ? latA : latB).push_back(r.latAvg);
            (algo == Algo::NF ? thrA : thrB).push_back(r.throughput);
        }
    }
    const double t = TCrit975(runs - 1);
    bool allOk = true;
    for (auto [metric, a, b] : {std::tuple{"lat_avg", &latA, &latB},
                                std::tuple{"throughput", &thrA, &thrB}})
    {
        const Sample sa = Describe(*a);
        const Sample sb = Describe(*b);
        const double diff = sa.mean - sb.mean;
        const double bound = t * std::sqrt(sa.sd * sa.sd / runs + sb.sd * sb.sd / runs);
        const bool ok = std::abs(diff) <= bound;
        allOk = allOk && ok;
        os << "RESULT,mode=simcheck,check=mirror_nf_t1_vs_nfws_t2,kind=stat,pir=0.02,runs=" << runs
           << ",metric=" << metric << ",mean_nf_t1=" << Fixed(sa.mean)
           << ",sd_nf_t1=" << Fixed(sa.sd) << ",mean_nfws_t2=" << Fixed(sb.mean)
           << ",sd_nfws_t2=" << Fixed(sb.sd) << ",diff=" << Fixed(diff)
           << ",bound=" << Fixed(bound) << ",t=" << Fixed(t, 3) << ",df=" << runs - 1
           << ",alpha=0.05,status=" << (ok ? "PASS" : "FAIL") << "\n";
    }
    return allOk;
}

} // namespace

int
RunSimCheck(std::ostream& os)
{
    const RngRunGuard guard;
    int failures = 0;
    for (Pattern p : {Pattern::Transpose1, Pattern::Transpose2})
    {
        failures += CheckLowLoad(p, os);
    }

    os << "# Ghi chú: kiểm định mirror có α = 0.05 (t với df = n−1, bảo thủ hơn Welch), nên khoảng 5%\n"
       << "# số lần chạy có thể FAIL dù hai cấu hình tương đương. Nếu FAIL với RngRun 1..5 thì tự\n"
       << "# chạy lại với RngRun 1..10 trước khi kết luận.\n";
    bool mirrorOk = CheckMirror(5, os);
    if (!mirrorOk)
    {
        os << "# Mirror FAIL với RngRun 1..5 → chạy lại với RngRun 1..10\n";
        mirrorOk = CheckMirror(10, os);
    }
    failures += mirrorOk ? 0 : 1;
    os << "RESULT,mode=simcheck,check=summary,failures=" << failures << "\n";
    return failures;
}

} // namespace noc
