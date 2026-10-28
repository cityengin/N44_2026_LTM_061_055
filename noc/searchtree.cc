#include "searchtree.h"

#include <bit>
#include <stdexcept>
#include <string>

namespace noc
{
namespace
{

class Walker
{
  public:
    Walker(Algo algo, const Mesh& mesh, int src, int dst, const PathFn& fn)
        : m_algo(algo),
          m_mesh(mesh),
          m_src(mesh.ToCoord(src)),
          m_dst(dst),
          m_dstCoord(mesh.ToCoord(dst)),
          m_fn(fn)
    {
    }

    void Visit(int cur, double prob)
    {
        m_nodes.push_back(cur);
        if (cur == m_dst)
        {
            m_fn(m_nodes, prob);
        }
        else
        {
            const DirSet s = routeSet(m_algo, m_mesh.ToCoord(cur), m_src, m_dstCoord);
            const int n = std::popcount(static_cast<unsigned>(s));
            if (n == 0 || (s & ~kAllLinkDirs) != 0)
            {
                Fail("tập hướng không hợp lệ tại node " + std::to_string(cur) +
                     " (set=" + std::to_string(s) + ")");
            }
            const int dist = m_mesh.Distance(cur, m_dst);
            for (Dir d : kDirOrder)
            {
                if (!Has(s, d))
                {
                    continue;
                }
                const int next = m_mesh.Neighbor(cur, d);
                if (next < 0 || m_mesh.Distance(next, m_dst) != dist - 1)
                {
                    Fail(std::string("hướng ") + DirName(d) + " tại node " + std::to_string(cur) +
                         " ra ngoài lưới hoặc không minimal");
                }
                Visit(next, prob / n);
            }
        }
        m_nodes.pop_back();
    }

  private:
    [[noreturn]] void Fail(const std::string& what) const
    {
        throw std::logic_error(std::string("ForEachPath ") + AlgoName(m_algo) + " " +
                               std::to_string(m_nodes.front()) + "->" + std::to_string(m_dst) +
                               ": " + what);
    }

    Algo m_algo;
    const Mesh& m_mesh;
    Coord m_src;
    int m_dst;
    Coord m_dstCoord;
    const PathFn& m_fn;
    std::vector<int> m_nodes;
};

} // namespace

void
ForEachPath(Algo algo, const Mesh& mesh, int src, int dst, const PathFn& fn)
{
    mesh.ToCoord(dst); // kiểm tra dst hợp lệ
    Walker(algo, mesh, src, dst, fn).Visit(src, 1.0);
}

long
CountPaths(Algo algo, const Mesh& mesh, int src, int dst)
{
    long count = 0;
    ForEachPath(algo, mesh, src, dst, [&count](const std::vector<int>&, double) { ++count; });
    return count;
}

} // namespace noc
