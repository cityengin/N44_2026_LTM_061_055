#include "selftest.h"

#include "routing.h"
#include "searchtree.h"
#include "topology.h"

#include <cmath>
#include <cstdlib>
#include <exception>
#include <iostream>
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
    if (IsDiagnostic(algo))
    {
        tag += ",variant=diagnostic";
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

} // namespace

int
RunSelfTest(bool forceFail)
{
    g_hardFailures = 0;
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

    if (forceFail)
    {
        Check("forced_fail", false, "reason=selftest-force-fail");
    }
    std::cout << "RESULT,mode=selftest,test=summary,hard_failures=" << g_hardFailures << "\n";
    return g_hardFailures;
}

} // namespace noc
