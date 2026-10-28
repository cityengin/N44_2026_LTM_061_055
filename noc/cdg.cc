#include "cdg.h"

#include "searchtree.h"

#include <algorithm>

namespace noc
{
namespace
{

enum Color : char
{
    White, // chưa thăm
    Gray,  // đang trên đường DFS
    Black, // đã xong
};

class CycleFinder
{
  public:
    explicit CycleFinder(const std::vector<std::vector<int>>& adj)
        : m_adj(adj),
          m_color(adj.size(), White)
    {
    }

    std::vector<int> Find()
    {
        for (std::size_t u = 0; u < m_adj.size() && m_cycle.empty(); ++u)
        {
            if (m_color[u] == White)
            {
                Visit(static_cast<int>(u));
            }
        }
        return m_cycle;
    }

  private:
    bool Visit(int u)
    {
        m_color[u] = Gray;
        m_stack.push_back(u);
        for (int v : m_adj[u])
        {
            if (m_color[v] == Gray)
            {
                // Cạnh ngược: chu trình là đoạn từ v tới u trên đường DFS.
                m_cycle.assign(std::find(m_stack.begin(), m_stack.end(), v), m_stack.end());
                return true;
            }
            if (m_color[v] == White && Visit(v))
            {
                return true;
            }
        }
        m_stack.pop_back();
        m_color[u] = Black;
        return false;
    }

    const std::vector<std::vector<int>>& m_adj;
    std::vector<char> m_color;
    std::vector<int> m_stack;
    std::vector<int> m_cycle;
};

} // namespace

CdgResult
CheckCdg(Algo algo, const Mesh& mesh)
{
    const int n = mesh.NumChannels();
    std::vector<char> has(static_cast<std::size_t>(n) * n, 0);
    std::vector<std::vector<int>> adj(n);
    CdgResult r{n, 0, 0, true, {}};
    for (int src = 0; src < mesh.NumNodes(); ++src)
    {
        for (int dst = 0; dst < mesh.NumNodes(); ++dst)
        {
            if (src == dst)
            {
                continue;
            }
            ForEachPath(algo, mesh, src, dst, [&](const std::vector<int>& nodes, double) {
                int prev = -1;
                for (std::size_t i = 0; i + 1 < nodes.size(); ++i)
                {
                    const int ch = mesh.ChannelBetween(nodes[i], nodes[i + 1]);
                    if (prev >= 0 && !has[static_cast<std::size_t>(prev) * n + ch])
                    {
                        has[static_cast<std::size_t>(prev) * n + ch] = 1;
                        adj[prev].push_back(ch);
                        ++r.edges;
                        if (mesh.GetChannel(prev).dir != mesh.GetChannel(ch).dir)
                        {
                            ++r.turnEdges;
                        }
                    }
                    prev = ch;
                }
            });
        }
    }
    r.cycle = CycleFinder(adj).Find();
    r.acyclic = r.cycle.empty();
    return r;
}

std::string
CycleString(const Mesh& mesh, const std::vector<int>& cycle)
{
    std::string s;
    for (int ch : cycle)
    {
        s += (s.empty() ? "" : ">") + mesh.ChannelName(ch);
    }
    return s;
}

} // namespace noc
