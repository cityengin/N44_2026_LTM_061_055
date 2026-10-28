#include "selftest.h"

#include "topology.h"

#include <iostream>
#include <string>

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

} // namespace

int
RunSelfTest()
{
    g_hardFailures = 0;
    TestIdCoord();
    TestChannelCount();
    TestNeighbors();
    TestChannelConsistency();
    std::cout << "RESULT,mode=selftest,test=summary,hard_failures=" << g_hardFailures << "\n";
    return g_hardFailures;
}

} // namespace noc
