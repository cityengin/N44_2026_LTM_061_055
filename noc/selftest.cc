#include "selftest.h"

#include "cdg.h"
#include "format.h"
#include "noc-header.h"
#include "noc-sim.h"
#include "phi.h"
#include "routing.h"
#include "searchtree.h"
#include "selection.h"
#include "timestamp-tag.h"
#include "topology.h"
#include "traffic.h"

#include "ns3/data-rate.h"
#include "ns3/packet.h"
#include "ns3/random-variable-stream.h"
#include "ns3/rng-seed-manager.h"

#include <array>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace noc
{
namespace
{

int g_hardFailures = 0;
int g_softMatch = 0;
int g_softMismatch = 0;

// Test SOFT (so với số của bài báo): KHÔNG tính vào exit code.
void
Soft(const std::string& test, bool match, const std::string& detail)
{
    std::cout << "RESULT,mode=selftest,test=" << test
              << ",status=" << (match ? "MATCH" : "MISMATCH") << "," << detail << "\n";
    ++(match ? g_softMatch : g_softMismatch);
}

// Test HARD: sai thì tính vào exit code.
void
Check(const std::string& test, bool ok, const std::string& detail = "")
{
    std::cout << "RESULT,mode=selftest,test=" << test << ",status=" << (ok ? "PASS" : "FAIL");
    if (!detail.empty())
    {
        std::cout << "," << detail;
    }
    std::cout << "\n";
    if (!ok)
    {
        ++g_hardFailures;
    }
}

// Giữ dòng RESULT đọc được bằng máy: thông điệp lỗi không được chứa dấu phẩy.
std::string
NoCommas(std::string s)
{
    for (char& c : s)
    {
        if (c == ',')
        {
            c = ';';
        }
    }
    return s;
}

std::string
AlgoTag(Algo algo)
{
    std::string tag = std::string("routing=") + AlgoName(algo);
    const std::string variant = VariantName(algo);
    if (!variant.empty())
    {
        tag += ",variant=" + variant;
    }
    return tag;
}

std::string
MeshTag(const Mesh& m)
{
    return "mesh=" + std::to_string(m.W()) + "x" + std::to_string(m.H());
}

std::string
PathString(const std::vector<int>& nodes)
{
    std::string s;
    for (std::size_t i = 0; i < nodes.size(); ++i)
    {
        s += (i == 0 ? "" : ">") + std::to_string(nodes[i]);
    }
    return s;
}

long
Binom(int n, int k)
{
    long r = 1;
    for (int i = 1; i <= k; ++i)
    {
        r = r * (n - k + i) / i;
    }
    return r;
}

// R(x, y) = (x, H−1−y): lật trục dọc.
int
Mirror(const Mesh& m, int id)
{
    const Coord c = m.ToCoord(id);
    return m.ToId(c.x, m.H() - 1 - c.y);
}

const std::vector<Mesh>&
TestMeshes()
{
    static const std::vector<Mesh> meshes{Mesh(3, 3), Mesh(7, 7)};
    return meshes;
}

// ---------------------------------------------------------------- topology

void
TestIdCoord()
{
    bool ok = true;
    for (auto [w, h] : {std::pair{7, 7}, {3, 5}, {5, 3}, {1, 1}})
    {
        Mesh m(w, h);
        for (int id = 0; id < m.NumNodes(); ++id)
        {
            Coord c = m.ToCoord(id);
            ok = ok && c.y * w + c.x == id && m.ToId(c.x, c.y) == id;
        }
    }
    Mesh m(7, 7);
    // Gốc góc trên-trái, id tăng theo hàng (Fig. 14).
    ok = ok && m.ToCoord(6).x == 6 && m.ToCoord(6).y == 0;
    ok = ok && m.ToCoord(42).x == 0 && m.ToCoord(42).y == 6;
    ok = ok && m.Distance(0, 48) == 12 && m.Distance(10, 26) == 4;
    Check("topo_idcoord", ok);
}

void
TestChannelCount()
{
    for (auto [w, h, expect] : {std::tuple{2, 2, 8}, {3, 3, 24}, {7, 7, 168}, {3, 5, 44}, {1, 1, 0}})
    {
        Mesh m(w, h);
        Check("topo_channels_" + std::to_string(w) + "x" + std::to_string(h),
              m.NumChannels() == expect,
              "got=" + std::to_string(m.NumChannels()) + ",expect=" + std::to_string(expect));
    }
}

void
TestNeighbors()
{
    Mesh m(7, 7);
    bool ok = m.Neighbor(0, Dir::N) == -1 && m.Neighbor(0, Dir::W) == -1 &&
              m.Neighbor(0, Dir::E) == 1 && m.Neighbor(0, Dir::S) == 7 &&
              m.Neighbor(48, Dir::E) == -1 && m.Neighbor(48, Dir::S) == -1 &&
              m.Neighbor(24, Dir::N) == 17 && m.Neighbor(24, Dir::W) == 23 &&
              m.Neighbor(6, Dir::E) == -1 && m.Neighbor(7, Dir::W) == -1 &&
              m.Neighbor(24, Dir::Local) == -1;
    Check("topo_neighbors", ok);
}

void
TestChannelConsistency()
{
    bool ok = true;
    for (auto [w, h] : {std::pair{7, 7}, {3, 5}})
    {
        Mesh m(w, h);
        for (int ch = 0; ch < m.NumChannels(); ++ch)
        {
            const Channel& c = m.GetChannel(ch);
            ok = ok && m.Neighbor(c.u, c.dir) == c.v && m.ChannelId(c.u, c.dir) == ch &&
                 m.Neighbor(c.v, Opposite(c.dir)) == c.u && m.Distance(c.u, c.v) == 1;
        }
    }
    Mesh m(7, 7);
    // Kênh 17-24 trong mục 5.3 của bài báo là link hướng Nam.
    int ch = m.ChannelId(17, Dir::S);
    ok = ok && ch >= 0 && m.ChannelName(ch) == "17-24" && m.ChannelId(0, Dir::N) == -1;
    Check("topo_channel_consistency", ok);
}

// ---------------------------------------------------------------- routing

void
TestBitOrder()
{
    bool ok = Bit(Dir::E) == 1 && Bit(Dir::W) == 2 && Bit(Dir::N) == 4 && Bit(Dir::S) == 8 &&
              kDirOrder[0] == Dir::E && kDirOrder[1] == Dir::W && kDirOrder[2] == Dir::N &&
              kDirOrder[3] == Dir::S;
    Check("route_bit_order", ok, "order=E>W>N>S");
}

void
TestAlgoNames()
{
    bool ok = true;
    for (Algo a : kAllAlgos)
    {
        ok = ok && ParseAlgo(AlgoName(a)) == a && IsDiagnostic(a) == (a == Algo::NF_WS);
    }
    bool threw = false;
    try
    {
        ParseAlgo("xy");
    }
    catch (const std::invalid_argument&)
    {
        threw = true;
    }
    Check("algo_names", ok && threw);
}

// (a), (b): chỉ xét các node đi tới được từ src khi đi theo routeSet.
void
TestReachableStates(Algo algo, const Mesh& m)
{
    long pairs = 0;
    long states = 0;
    std::string emptyFail;
    std::string minimalFail;
    for (int src = 0; src < m.NumNodes(); ++src)
    {
        for (int dst = 0; dst < m.NumNodes(); ++dst)
        {
            if (src == dst)
            {
                continue;
            }
            ++pairs;
            std::vector<char> seen(m.NumNodes(), 0);
            std::vector<int> stack{src};
            seen[src] = 1;
            while (!stack.empty())
            {
                const int cur = stack.back();
                stack.pop_back();
                ++states;
                const DirSet s = routeSet(algo, m.ToCoord(cur), m.ToCoord(src), m.ToCoord(dst));
                const std::string where = "src=" + std::to_string(src) +
                                          ",dst=" + std::to_string(dst) +
                                          ",cur=" + std::to_string(cur) +
                                          ",set=" + std::to_string(s);
                if ((s == 0) != (cur == dst) && emptyFail.empty())
                {
                    emptyFail = where;
                }
                if ((s & ~kAllLinkDirs) != 0 && minimalFail.empty())
                {
                    minimalFail = where + ",reason=bit_ngoai_E_W_N_S";
                }
                for (Dir d : kDirOrder)
                {
                    if (!Has(s, d))
                    {
                        continue;
                    }
                    const int next = m.Neighbor(cur, d);
                    if (next < 0)
                    {
                        if (minimalFail.empty())
                        {
                            minimalFail = where + ",dir=" + DirName(d) + ",reason=ra_ngoai_luoi";
                        }
                        continue;
                    }
                    if (m.Distance(next, dst) != m.Distance(cur, dst) - 1)
                    {
                        if (minimalFail.empty())
                        {
                            minimalFail = where + ",dir=" + DirName(d) + ",reason=khong_giam_1";
                        }
                        continue;
                    }
                    if (!seen[next])
                    {
                        seen[next] = 1;
                        stack.push_back(next);
                    }
                }
            }
        }
    }
    const std::string base = AlgoTag(algo) + "," + MeshTag(m) + ",pairs=" + std::to_string(pairs) +
                             ",states=" + std::to_string(states);
    Check("route_empty_iff_dst", emptyFail.empty(), base + (emptyFail.empty() ? "" : "," + emptyFail));
    Check("route_minimal_in_mesh",
          minimalFail.empty(),
          base + (minimalFail.empty() ? "" : "," + minimalFail));
}

// Bảng turn cấm viết riêng, KHÔNG gọi routeSet. column: −1 mọi cột, 0 cột chẵn, 1 cột lẻ.
// Turn in→out được xét tại node nơi rẽ.
struct TurnRule
{
    Algo algo;
    Dir in;
    Dir out;
    int column;
};

const TurnRule kForbiddenTurns[] = {
    {Algo::XY, Dir::N, Dir::E, -1},
    {Algo::XY, Dir::N, Dir::W, -1},
    {Algo::XY, Dir::S, Dir::E, -1},
    {Algo::XY, Dir::S, Dir::W, -1},
    {Algo::YX, Dir::E, Dir::N, -1},
    {Algo::YX, Dir::E, Dir::S, -1},
    {Algo::YX, Dir::W, Dir::N, -1},
    {Algo::YX, Dir::W, Dir::S, -1},
    {Algo::NF, Dir::E, Dir::N, -1},
    {Algo::NF, Dir::S, Dir::W, -1},
    {Algo::NF_WS, Dir::E, Dir::S, -1},
    {Algo::NF_WS, Dir::N, Dir::W, -1},
    {Algo::OE, Dir::E, Dir::N, 0},
    {Algo::OE, Dir::E, Dir::S, 0},
    {Algo::OE, Dir::N, Dir::W, 1},
    {Algo::OE, Dir::S, Dir::W, 1},
};

bool
HasTurnRules(Algo algo)
{
    for (const TurnRule& r : kForbiddenTurns)
    {
        if (r.algo == algo)
        {
            return true;
        }
    }
    return false;
}

bool
IsForbiddenTurn(Algo algo, Dir in, Dir out, int x)
{
    for (const TurnRule& r : kForbiddenTurns)
    {
        if (r.algo == algo && r.in == in && r.out == out && (r.column < 0 || r.column == x % 2))
        {
            return true;
        }
    }
    return false;
}

// Hướng đi suy trực tiếp từ tọa độ hai node liên tiếp.
Dir
MoveDir(Coord a, Coord b)
{
    if (b.y == a.y && b.x == a.x + 1)
    {
        return Dir::E;
    }
    if (b.y == a.y && b.x == a.x - 1)
    {
        return Dir::W;
    }
    if (b.x == a.x && b.y == a.y - 1)
    {
        return Dir::N;
    }
    if (b.x == a.x && b.y == a.y + 1)
    {
        return Dir::S;
    }
    throw std::logic_error("MoveDir: hai node không kề nhau");
}

// Đi dọc một đường, đếm số turn vào turns. Trả về mô tả turn cấm đầu tiên theo bảng
// của tableAlgo, hoặc chuỗi rỗng nếu không có.
std::string
ScanTurns(Algo tableAlgo, const Mesh& m, const std::vector<int>& nodes, long& turns)
{
    std::string fail;
    for (std::size_t i = 1; i + 1 < nodes.size(); ++i)
    {
        const Coord at = m.ToCoord(nodes[i]);
        const Dir in = MoveDir(m.ToCoord(nodes[i - 1]), at);
        const Dir out = MoveDir(at, m.ToCoord(nodes[i + 1]));
        if (in == out)
        {
            continue;
        }
        ++turns;
        if (fail.empty() && IsForbiddenTurn(tableAlgo, in, out, at.x))
        {
            fail = std::string("turn=") + DirName(in) + ">" + DirName(out) +
                   ",at=" + std::to_string(nodes[i]) + ",x=" + std::to_string(at.x) +
                   ",path=" + PathString(nodes);
        }
    }
    return fail;
}

// Số đường kỳ vọng theo (dx, dy) = dst − src; −1 nếu không có công thức (OE).
long
ExpectedPaths(Algo algo, int dx, int dy)
{
    const long c = Binom(std::abs(dx) + std::abs(dy), std::abs(dx));
    const long sign = static_cast<long>(dx) * dy; // < 0: trái dấu, > 0: cùng dấu
    switch (algo)
    {
    case Algo::XY:
    case Algo::YX:
        return 1;
    case Algo::FULL:
        return c;
    case Algo::NF:
        return sign < 0 ? 1 : c;
    case Algo::NF_WS:
        return sign > 0 ? 1 : c;
    case Algo::OE:
        return -1;
    }
    return -1;
}

// (c), (d), (e) và Σ hopProb = 1, từ cùng một lần liệt kê đường cho mọi cặp src≠dst.
void
TestPaths(Algo algo, const Mesh& m)
{
    long pairs = 0;
    long paths = 0;
    long turns = 0;
    std::string lenFail;
    std::string countFail;
    std::string turnFail;
    std::string hopFail;
    std::string error;
    try
    {
        for (int src = 0; src < m.NumNodes(); ++src)
        {
            for (int dst = 0; dst < m.NumNodes(); ++dst)
            {
                if (src == dst)
                {
                    continue;
                }
                ++pairs;
                long count = 0;
                double hopSum = 0.0;
                const std::string pair = "src=" + std::to_string(src) + ",dst=" + std::to_string(dst);
                ForEachPath(algo, m, src, dst, [&](const std::vector<int>& nodes, double prob) {
                    ++count;
                    hopSum += prob;
                    const int len = static_cast<int>(nodes.size()) - 1;
                    if (len != m.Distance(src, dst) && lenFail.empty())
                    {
                        lenFail = pair + ",len=" + std::to_string(len) + ",path=" + PathString(nodes);
                    }
                    const std::string bad = ScanTurns(algo, m, nodes, turns);
                    if (!bad.empty() && turnFail.empty())
                    {
                        turnFail = pair + "," + bad;
                    }
                });
                paths += count;
                const Coord s = m.ToCoord(src);
                const Coord d = m.ToCoord(dst);
                const long expect = ExpectedPaths(algo, d.x - s.x, d.y - s.y);
                if (expect >= 0 && count != expect && countFail.empty())
                {
                    countFail =
                        pair + ",got=" + std::to_string(count) + ",expect=" + std::to_string(expect);
                }
                if (std::abs(hopSum - 1.0) > 1e-9 && hopFail.empty())
                {
                    hopFail = pair + ",sum=" + std::to_string(hopSum);
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        error = "exception=" + NoCommas(e.what());
    }

    auto report = [&](const std::string& test, const std::string& fail, const std::string& extra) {
        const std::string& why = error.empty() ? fail : error;
        Check(test,
              why.empty(),
              AlgoTag(algo) + "," + MeshTag(m) + ",pairs=" + std::to_string(pairs) +
                  ",paths=" + std::to_string(paths) + extra + (why.empty() ? "" : "," + why));
    };
    report("path_len_manhattan", lenFail, "");
    if (ExpectedPaths(algo, 1, 1) >= 0)
    {
        report("path_count_all_pairs", countFail, "");
    }
    if (HasTurnRules(algo))
    {
        report("turn_rules", turnFail, ",turns=" + std::to_string(turns));
    }
    report("hop_prob_sum", hopFail, "");
}

// Đối chứng âm cho turn_rules: đường của FULL phải vi phạm bảng turn cấm của algo,
// nếu không thì bảng (hoặc cách quét turn) không bắt được gì và turn_rules vô nghĩa.
void
TestTurnRulesNegative(Algo algo)
{
    Mesh m(7, 7);
    long turns = 0;
    long violating = 0;
    for (int src = 0; src < m.NumNodes(); ++src)
    {
        for (int dst = 0; dst < m.NumNodes(); ++dst)
        {
            if (src == dst)
            {
                continue;
            }
            ForEachPath(Algo::FULL, m, src, dst, [&](const std::vector<int>& nodes, double) {
                if (!ScanTurns(algo, m, nodes, turns).empty())
                {
                    ++violating;
                }
            });
        }
    }
    Check("turn_rules_negative_control",
          violating > 0,
          AlgoTag(algo) + ",paths_from=FULL," + MeshTag(m) +
              ",violating_paths=" + std::to_string(violating));
}

// Đối xứng qua R: số đường NF(s,d) == số đường NF_WS(R(s),R(d)) với mọi cặp.
void
TestMirrorPathCounts(const Mesh& m)
{
    long pairs = 0;
    std::string fail;
    try
    {
        for (int src = 0; src < m.NumNodes(); ++src)
        {
            for (int dst = 0; dst < m.NumNodes(); ++dst)
            {
                if (src == dst)
                {
                    continue;
                }
                ++pairs;
                const long a = CountPaths(Algo::NF, m, src, dst);
                const long b = CountPaths(Algo::NF_WS, m, Mirror(m, src), Mirror(m, dst));
                if (a != b && fail.empty())
                {
                    fail = "src=" + std::to_string(src) + ",dst=" + std::to_string(dst) +
                           ",nf=" + std::to_string(a) + ",nf_ws_mirror=" + std::to_string(b);
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        fail = "exception=" + NoCommas(e.what());
    }
    Check("mirror_path_count_nf_nfws",
          fail.empty(),
          MeshTag(m) + ",pairs=" + std::to_string(pairs) + (fail.empty() ? "" : "," + fail));
}

// Mốc của bài báo: số đường 0→48 trên 7×7. Nếu sai thì in thêm vài đường mẫu.
void
TestPaths0To48(Algo algo, long expect)
{
    Mesh m(7, 7);
    long count = 0;
    std::string samples;
    std::string error;
    try
    {
        ForEachPath(algo, m, 0, 48, [&](const std::vector<int>& nodes, double) {
            if (++count <= 3)
            {
                samples += (samples.empty() ? "" : ";") + PathString(nodes);
            }
        });
    }
    catch (const std::exception& e)
    {
        error = ",exception=" + NoCommas(e.what());
    }
    const bool ok = error.empty() && count == expect;
    Check("paths_0_48",
          ok,
          AlgoTag(algo) + "," + MeshTag(m) + ",got=" + std::to_string(count) +
              ",expect=" + std::to_string(expect) + (ok ? "" : ",samples=" + samples + error));
}

// ---------------------------------------------------------------- traffic, phi, cdg

void
TestTrafficFlows()
{
    Mesh m(7, 7);
    const std::vector<Flow> un = MakeFlows(Pattern::Uniform, m);
    const std::vector<Flow> t1 = MakeFlows(Pattern::Transpose1, m);
    const std::vector<Flow> t2 = MakeFlows(Pattern::Transpose2, m);
    bool ok = un.size() == 2352 && t1.size() == 42 && t2.size() == 42;
    bool has10to26 = false; // ví dụ transpose1 ở mục 5.3 của bài báo
    for (const Flow& f : t1)
    {
        const Coord s = m.ToCoord(f.src);
        const Coord d = m.ToCoord(f.dst);
        ok = ok && f.src != f.dst && f.weight == 1.0 && d.x == 6 - s.y && d.y == 6 - s.x;
        has10to26 = has10to26 || (f.src == 10 && f.dst == 26);
    }
    for (const Flow& f : t2)
    {
        const Coord s = m.ToCoord(f.src);
        const Coord d = m.ToCoord(f.dst);
        ok = ok && f.src != f.dst && f.weight == 1.0 && d.x == s.y && d.y == s.x;
    }
    bool threw = false;
    try
    {
        MakeFlows(Pattern::Transpose1, Mesh(3, 5));
    }
    catch (const std::invalid_argument&)
    {
        threw = true;
    }
    Check("traffic_flows",
          ok && has10to26 && threw,
          MeshTag(m) + ",uniform=" + std::to_string(un.size()) + ",transpose1=" +
              std::to_string(t1.size()) + ",transpose2=" + std::to_string(t2.size()));
}

// ρ được tính một lần cho mỗi (routing, traffic, lưới) rồi dùng lại giữa các test.
const ChannelPressure&
Pressure(Algo algo, Pattern p, const Mesh& m)
{
    static std::map<std::tuple<int, int, int, int>, ChannelPressure> cache;
    const auto key = std::make_tuple(static_cast<int>(algo), static_cast<int>(p), m.W(), m.H());
    auto it = cache.find(key);
    if (it == cache.end())
    {
        it = cache.emplace(key, ComputePressure(algo, m, MakeFlows(p, m))).first;
    }
    return it->second;
}

// Bảo toàn: Σ_kênh ρ = Σ_cặp (trọng số × khoảng cách Manhattan), mọi traffic, cả 2 phương pháp.
void
TestConservation(Algo algo, const Mesh& m)
{
    std::string fail;
    int checked = 0;
    for (Pattern p : kAllPatterns)
    {
        double expect = 0.0;
        for (const Flow& f : MakeFlows(p, m))
        {
            expect += f.weight * m.Distance(f.src, f.dst);
        }
        for (Method meth : kAllMethods)
        {
            double sum = 0.0;
            for (double v : Pressure(algo, p, m).Get(meth))
            {
                sum += v;
            }
            ++checked;
            if (std::abs(sum - expect) > 1e-6 && fail.empty())
            {
                fail = std::string(",traffic=") + PatternName(p) + ",method=" + MethodName(meth) +
                       ",sum_rho=" + Fixed(sum) + ",expect=" + Fixed(expect);
            }
        }
    }
    Check("conservation",
          fail.empty(),
          AlgoTag(algo) + "," + MeshTag(m) + ",checked=" + std::to_string(checked) + fail);
}

// Routing tất định với traffic này: path == hop trên mọi kênh và mọi ρ là số nguyên.
void
TestDeterministicInteger(Algo algo, Pattern p, const Mesh& m)
{
    const ChannelPressure& cp = Pressure(algo, p, m);
    std::string fail;
    for (int ch = 0; ch < m.NumChannels() && fail.empty(); ++ch)
    {
        const double a = cp.path[ch];
        const double b = cp.hop[ch];
        if (std::abs(a - b) > 1e-9 || std::abs(a - std::round(a)) > 1e-9)
        {
            fail = ",ch=" + m.ChannelName(ch) + ",path=" + Fixed(a) + ",hop=" + Fixed(b);
        }
    }
    Check("deterministic_integer",
          fail.empty(),
          AlgoTag(algo) + ",traffic=" + PatternName(p) + "," + MeshTag(m) + fail);
}

int
MirrorChannel(const Mesh& m, int ch)
{
    const Channel& c = m.GetChannel(ch);
    return m.ChannelBetween(Mirror(m, c.u), Mirror(m, c.v));
}

// Đối xứng cấp kênh qua R: ρ(a, pa, R(ch)) == ρ(b, pb, ch) với mọi kênh, cả 2 phương pháp.
void
TestMirrorPressure(Algo a, Pattern pa, Algo b, Pattern pb, const Mesh& m)
{
    std::string fail;
    for (Method meth : kAllMethods)
    {
        const std::vector<double>& ra = Pressure(a, pa, m).Get(meth);
        const std::vector<double>& rb = Pressure(b, pb, m).Get(meth);
        for (int ch = 0; ch < m.NumChannels() && fail.empty(); ++ch)
        {
            const int rch = MirrorChannel(m, ch);
            if (std::abs(ra[rch] - rb[ch]) > 1e-9)
            {
                fail = std::string(",method=") + MethodName(meth) + ",ch=" + m.ChannelName(ch) +
                       ",lhs=" + Fixed(ra[rch]) + ",rhs=" + Fixed(rb[ch]);
            }
        }
    }
    const bool diag = IsDiagnostic(a) || IsDiagnostic(b);
    Check("mirror_channel_pressure",
          fail.empty(),
          std::string("lhs=") + AlgoName(a) + ":" + PatternName(pa) + ":R(ch),rhs=" + AlgoName(b) +
              ":" + PatternName(pb) + ":ch" + (diag ? ",variant=diagnostic" : "") + "," +
              MeshTag(m) + fail);
}

// FULL phải có chu trình (đối chứng âm); các thuật toán còn lại phải không có chu trình.
void
TestCdg(Algo algo, const Mesh& m)
{
    const bool expectAcyclic = algo != Algo::FULL;
    const CdgResult r = CheckCdg(algo, m);
    Check(expectAcyclic ? "cdg_acyclic" : "cdg_cycle_negative_control",
          r.acyclic == expectAcyclic,
          AlgoTag(algo) + "," + MeshTag(m) + ",channels=" + std::to_string(r.channels) +
              ",edges=" + std::to_string(r.edges) + ",turn_edges=" + std::to_string(r.turnEdges) +
              ",acyclic=" + (r.acyclic ? "1" : "0") +
              (r.cycle.empty() ? "" : ",cycle=" + CycleString(m, r.cycle)));
}

// SOFT: so φ với số của bài báo (±0.005). NF_WS được so với số của NF trong bài báo.
void
TestPaperPhi()
{
    struct Ref
    {
        Algo algo;
        Pattern p;
        double paper;
    };

    const Ref refs[] = {
        {Algo::XY, Pattern::Transpose1, 6.0},
        {Algo::OE, Pattern::Transpose1, 4.81},
        {Algo::NF, Pattern::Transpose1, 6.0},
        {Algo::NF_WS, Pattern::Transpose1, 6.0},
        {Algo::XY, Pattern::Transpose2, 6.0},
        {Algo::OE, Pattern::Transpose2, 4.81},
        {Algo::NF, Pattern::Transpose2, 2.41},
        {Algo::NF_WS, Pattern::Transpose2, 2.41},
    };
    const Mesh m7(7, 7);
    for (const Ref& r : refs)
    {
        for (Method meth : kAllMethods)
        {
            const double phi = MaxPressure(Pressure(r.algo, r.p, m7).Get(meth)).phi;
            Soft("paper_phi",
                 std::abs(phi - r.paper) <= 0.005,
                 AlgoTag(r.algo) + ",traffic=" + PatternName(r.p) + ",method=" + MethodName(meth) +
                     "," + MeshTag(m7) + ",got=" + Fixed(phi) + ",paper=" + Short(r.paper) +
                     (r.algo == Algo::NF_WS ? ",paper_ref=NF" : "") + ",tol=0.005");
        }
    }
    const Mesh m3(3, 3);
    for (Method meth : kAllMethods)
    {
        const double phi = MaxPressure(Pressure(Algo::XY, Pattern::Uniform, m3).Get(meth)).phi;
        Soft("paper_phi",
             std::abs(phi - 6.0) <= 0.005,
             std::string("routing=XY,traffic=uniform,method=") + MethodName(meth) + "," +
                 MeshTag(m3) + ",normalized=0,got=" + Fixed(phi) + ",paper=6,tol=0.005");
    }
}

// ---------------------------------------------------------------- mô phỏng: thành phần

void
TestFrameLayout()
{
    const uint32_t hdr = NoCHeader().GetSerializedSize();
    const bool ok = hdr == 10 && PayloadBytes() == 20 &&
                    kPppHeaderBytes + hdr + PayloadBytes() == kFrameBytes;
    Check("frame_layout",
          ok,
          "ppp=" + std::to_string(kPppHeaderBytes) + ",header=" + std::to_string(hdr) +
              ",payload=" + std::to_string(PayloadBytes()) + ",frame=" + std::to_string(kFrameBytes));
}

void
TestHeaderTagRoundtrip()
{
    ns3::Ptr<ns3::Packet> p = ns3::Create<ns3::Packet>(PayloadBytes());
    NoCHeader h(12, 34, 567890);
    h.SetHops(7);
    p->AddHeader(h);
    p->AddPacketTag(CreationTimeTag(123456789));
    // Tag không làm tăng số byte; PPP thêm 2 B khi gửi.
    const bool sizeOk = p->GetSize() == kFrameBytes - kPppHeaderBytes;
    ns3::Ptr<ns3::Packet> q = p->Copy();
    NoCHeader r;
    q->RemoveHeader(r);
    CreationTimeTag t;
    const bool tagOk = q->PeekPacketTag(t) && t.GetCreatedNs() == 123456789;
    const bool ok = sizeOk && tagOk && r.GetSrc() == 12 && r.GetDst() == 34 &&
                    r.GetSeq() == 567890 && r.GetHops() == 7;
    Check("header_tag_roundtrip", ok, "size_before_ppp=" + std::to_string(p->GetSize()));
}

void
TestNthDir()
{
    const DirSet ws = Bit(Dir::W) | Bit(Dir::S);
    bool ok = NthDir(kAllLinkDirs, 0) == Dir::E && NthDir(kAllLinkDirs, 1) == Dir::W &&
              NthDir(kAllLinkDirs, 2) == Dir::N && NthDir(kAllLinkDirs, 3) == Dir::S &&
              NthDir(ws, 0) == Dir::W && NthDir(ws, 1) == Dir::S && CountDirs(ws) == 2;
    int threw = 0;
    for (int k : {-1, 2})
    {
        try
        {
            NthDir(ws, k);
        }
        catch (const std::out_of_range&)
        {
            ++threw;
        }
    }
    Check("select_nth_dir", ok && threw == 2);
}

void
TestSelectUniform()
{
    auto rv = ns3::CreateObject<ns3::UniformRandomVariable>();
    rv->SetStream(1000000); // ngoài dải stream của mô phỏng (3·id + 0..2)
    const int n = 40000;
    std::array<int, 4> count{};
    for (int i = 0; i < n; ++i)
    {
        ++count[static_cast<int>(SelectUniform(kAllLinkDirs, rv))];
    }
    bool ok = true;
    for (int c : count)
    {
        ok = ok && std::abs(c - n / 4) <= n / 4 / 20; // ±5%
    }
    const DirSet ns = Bit(Dir::N) | Bit(Dir::S);
    for (int i = 0; i < 1000; ++i)
    {
        ok = ok && Has(ns, SelectUniform(ns, rv));
    }
    Check("select_uniform",
          ok,
          "n=" + std::to_string(n) + ",E=" + std::to_string(count[0]) +
              ",W=" + std::to_string(count[1]) + ",N=" + std::to_string(count[2]) +
              ",S=" + std::to_string(count[3]));
}

// ---------------------------------------------------------------- mô phỏng ns-3

// Một gói duy nhất 0→48 với XY: 12 hop × 16 ns = 192 ns. Nếu sai thì in thời gian từng hop
// đo được và thời gian truyền 32 B mà DataRate tính ra (nghi làm tròn). Không nới sai số.
void
TestSimSinglePacket()
{
    SimConfig cfg;
    cfg.algo = Algo::XY;
    cfg.warmup = 0;
    cfg.measure = 1;
    cfg.drain = 10000;
    cfg.injections = {{0, 0, 48}};
    const SimResult r = RunSim(cfg);
    const bool ok = r.generated == 1 && r.delivered == 1 && r.latAvg == 192.0 && r.hopMismatch == 0;
    std::string detail = "routing=XY,src=0,dst=48,hops=" + Short(r.hopsAvg) +
                         ",latency=" + Short(r.latAvg) + ",expect=192";
    if (!ok)
    {
        std::string perHop;
        for (std::size_t i = 0; i < r.txStartNs.size(); ++i)
        {
            const double next =
                i + 1 < r.txStartNs.size() ? r.txStartNs[i + 1] : r.latAvg; // tạo lúc 0
            perHop += (perHop.empty() ? "" : ";") + Short(next - r.txStartNs[i]);
        }
        const ns3::Time tx = ns3::DataRate("16Gbps").CalculateBytesTxTime(kFrameBytes);
        detail += ",per_hop_ns=" + perHop + ",tx_time_32B_ps=" + std::to_string(tx.GetPicoSeconds());
    }
    Check("sim_single_packet", ok, detail);
}

// Mọi frame trên dây đúng 32 B (PhyTxBegin cũng abort nếu sai).
void
TestSimFrame32()
{
    SimConfig cfg;
    cfg.algo = Algo::OE;
    cfg.warmup = 0;
    cfg.measure = 1;
    const Mesh m(cfg.w, cfg.h);
    for (const Flow& f : MakeFlows(Pattern::Transpose1, m))
    {
        cfg.injections.push_back({0, f.src, f.dst});
    }
    const SimResult r = RunSim(cfg);
    const bool ok = r.minFrame == kFrameBytes && r.maxFrame == kFrameBytes &&
                    r.delivered == r.generated && r.generated == 42 && r.drops == 0;
    Check("sim_frame_32",
          ok,
          "routing=OE,packets=" + std::to_string(r.generated) +
              ",min_frame=" + std::to_string(r.minFrame) + ",max_frame=" + std::to_string(r.maxFrame) +
              ",delivered=" + std::to_string(r.delivered) + ",drops=" + std::to_string(r.drops));
}

// Mọi gói tới đích có hops == khoảng cách Manhattan (uniform, PIR 0.01, cửa sổ ngắn).
void
TestSimHopsManhattan(Algo algo)
{
    SimConfig cfg;
    cfg.algo = algo;
    cfg.pattern = Pattern::Uniform;
    cfg.pir = 0.01;
    cfg.warmup = 0;
    cfg.measure = 2000;
    const SimResult r = RunSim(cfg);
    Check("sim_hops_manhattan",
          r.delivered > 0 && r.hopMismatch == 0,
          AlgoTag(algo) + ",traffic=uniform,pir=0.01,delivered=" + std::to_string(r.delivered) +
              ",hop_mismatch=" + std::to_string(r.hopMismatch) +
              ",hops_avg=" + Fixed(r.hopsAvg));
    // Từng gói: lat_i ≥ 16·hops_i; giá trị âm nghĩa là có bug.
    Check("sim_lat_ge_16hops",
          r.delivered > 0 && r.slackMin >= 0.0,
          AlgoTag(algo) + ",traffic=uniform,pir=0.01,slack_min=" + Short(r.slackMin) +
              ",slack_max=" + Short(r.slackMax));
}

// Phân vị nearest-rank với các vector đã biết kết quả.
void
TestPercentile()
{
    auto range = [](int from, int to) {
        std::vector<int64_t> v;
        for (int i = from; i <= to; ++i)
        {
            v.push_back(i);
        }
        return v;
    };
    // Mô phỏng p95 khi có k gói 12 hop (192) trong 100 gói, còn lại 10 hop (160).
    auto mix = [](int k) {
        std::vector<int64_t> v(100 - k, 160);
        v.insert(v.end(), k, 192);
        return v;
    };
    struct Case
    {
        const char* name;
        std::vector<int64_t> v;
        double q;
        double expect;
    };

    const std::vector<Case> cases = {
        {"1..100_q0.95", range(1, 100), 0.95, 95},
        {"1..100_q0.5", range(1, 100), 0.5, 50},
        {"1..100_q1", range(1, 100), 1.0, 100},
        {"1..100_q0.01", range(1, 100), 0.01, 1},
        {"1..20_q0.95", range(1, 20), 0.95, 19},
        {"1..4000_q0.95", range(1, 4000), 0.95, 3800},
        {"single", {7}, 0.95, 7},
        {"unsorted", {3, 1, 2}, 0.5, 2},
        {"5pct_192", mix(5), 0.95, 160},
        {"6pct_192", mix(6), 0.95, 192},
    };
    std::string fail;
    for (const Case& c : cases)
    {
        const double got = PercentileNearestRank(c.v, c.q);
        if (got != c.expect && fail.empty())
        {
            fail = std::string(",case=") + c.name + ",got=" + Short(got) + ",expect=" + Short(c.expect);
        }
    }
    const bool emptyNan = std::isnan(PercentileNearestRank({}, 0.95));
    Check("percentile_nearest_rank",
          fail.empty() && emptyNan,
          "cases=" + std::to_string(cases.size() + 1) + fail);
}

// Khôi phục RngRun ban đầu khi ra khỏi phạm vi, kể cả khi test fail hay ném exception.
class RngRunGuard
{
  public:
    RngRunGuard()
        : m_run(ns3::RngSeedManager::GetRun())
    {
    }

    ~RngRunGuard() { ns3::RngSeedManager::SetRun(m_run); }

    RngRunGuard(const RngRunGuard&) = delete;
    RngRunGuard& operator=(const RngRunGuard&) = delete;

  private:
    uint64_t m_run;
};

// Cùng RngRun → dòng RESULT giống hệt; khác RngRun → khác (đối chứng âm).
void
TestSimDeterministic()
{
    const RngRunGuard guard;
    SimConfig cfg;
    cfg.algo = Algo::OE;
    cfg.pattern = Pattern::Uniform;
    cfg.pir = 0.02;
    cfg.warmup = 200;
    cfg.measure = 2000;
    auto runWith = [&cfg](uint64_t run) {
        ns3::RngSeedManager::SetRun(run);
        return FormatResult(cfg, RunSim(cfg));
    };
    const std::string a = runWith(7);
    const std::string b = runWith(7);
    const std::string c = runWith(8);
    Check("sim_deterministic",
          a == b && a != c,
          std::string("routing=OE,traffic=uniform,run_same=7,run_other=8,same_equal=") +
              (a == b ? "1" : "0") + ",other_differs=" + (a != c ? "1" : "0"));
}

} // namespace

int
RunSelfTest(bool forceFail)
{
    g_hardFailures = 0;
    g_softMatch = 0;
    g_softMismatch = 0;
    TestIdCoord();
    TestChannelCount();
    TestNeighbors();
    TestChannelConsistency();

    TestBitOrder();
    TestAlgoNames();
    for (const Mesh& m : TestMeshes())
    {
        for (Algo a : kAllAlgos)
        {
            TestReachableStates(a, m);
        }
    }
    for (const Mesh& m : TestMeshes())
    {
        for (Algo a : kAllAlgos)
        {
            TestPaths(a, m);
        }
    }
    for (Algo a : kAllAlgos)
    {
        if (HasTurnRules(a))
        {
            TestTurnRulesNegative(a);
        }
    }
    for (const Mesh& m : TestMeshes())
    {
        TestMirrorPathCounts(m);
    }
    TestPaths0To48(Algo::FULL, 924);
    TestPaths0To48(Algo::OE, 84);
    TestPaths0To48(Algo::XY, 1);

    TestTrafficFlows();
    for (const Mesh& m : TestMeshes())
    {
        for (Algo a : kAllAlgos)
        {
            TestConservation(a, m);
        }
        for (Pattern p : kAllPatterns)
        {
            TestDeterministicInteger(Algo::XY, p, m);
            TestDeterministicInteger(Algo::YX, p, m);
        }
        TestDeterministicInteger(Algo::NF, Pattern::Transpose2, m);
        TestDeterministicInteger(Algo::NF_WS, Pattern::Transpose1, m);
        TestMirrorPressure(Algo::NF_WS, Pattern::Transpose1, Algo::NF, Pattern::Transpose2, m);
        TestMirrorPressure(Algo::NF_WS, Pattern::Transpose2, Algo::NF, Pattern::Transpose1, m);
        TestMirrorPressure(Algo::XY, Pattern::Transpose1, Algo::XY, Pattern::Transpose2, m);
        TestMirrorPressure(Algo::OE, Pattern::Transpose1, Algo::OE, Pattern::Transpose2, m);
        for (Algo a : kAllAlgos)
        {
            TestCdg(a, m);
        }
    }
    TestPaperPhi();

    TestFrameLayout();
    TestHeaderTagRoundtrip();
    TestNthDir();
    TestSelectUniform();
    TestPercentile();
    TestSimSinglePacket();
    TestSimFrame32();
    for (Algo a : {Algo::XY, Algo::YX, Algo::NF, Algo::NF_WS, Algo::OE})
    {
        TestSimHopsManhattan(a);
    }
    const uint64_t runBefore = ns3::RngSeedManager::GetRun();
    TestSimDeterministic();
    Check("sim_rngrun_restored",
          ns3::RngSeedManager::GetRun() == runBefore,
          "run=" + std::to_string(ns3::RngSeedManager::GetRun()) +
              ",expect=" + std::to_string(runBefore));

    if (forceFail)
    {
        Check("forced_fail", false, "reason=selftest-force-fail");
    }
    std::cout << "RESULT,mode=selftest,test=summary,hard_failures=" << g_hardFailures
              << ",soft_match=" << g_softMatch << ",soft_mismatch=" << g_softMismatch << "\n";
    return g_hardFailures;
}

} // namespace noc
