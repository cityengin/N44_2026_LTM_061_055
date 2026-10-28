#include "traffic.h"

#include <stdexcept>

namespace noc
{

std::vector<Flow>
MakeFlows(Pattern p, const Mesh& m)
{
    if (p != Pattern::Uniform && m.W() != m.H())
    {
        throw std::invalid_argument(std::string(PatternName(p)) + " cần lưới vuông (W == H)");
    }
    const int n = m.W(); // N trong công thức transpose
    std::vector<Flow> flows;
    for (int src = 0; src < m.NumNodes(); ++src)
    {
        if (p == Pattern::Uniform)
        {
            for (int dst = 0; dst < m.NumNodes(); ++dst)
            {
                if (dst != src)
                {
                    flows.push_back({src, dst, 1.0});
                }
            }
            continue;
        }
        const Coord c = m.ToCoord(src);
        const int dst =
            p == Pattern::Transpose1 ? m.ToId(n - 1 - c.y, n - 1 - c.x) : m.ToId(c.y, c.x);
        if (dst != src)
        {
            flows.push_back({src, dst, 1.0});
        }
    }
    return flows;
}

double
NormFactor(Pattern p, const Mesh& m)
{
    if (p == Pattern::Uniform && m.NumNodes() > 1)
    {
        return 1.0 / (m.NumNodes() - 1);
    }
    return 1.0;
}

const char*
PatternName(Pattern p)
{
    switch (p)
    {
    case Pattern::Uniform:
        return "uniform";
    case Pattern::Transpose1:
        return "transpose1";
    case Pattern::Transpose2:
        return "transpose2";
    }
    return "?";
}

Pattern
ParsePattern(const std::string& name)
{
    for (Pattern p : kAllPatterns)
    {
        if (name == PatternName(p))
        {
            return p;
        }
    }
    throw std::invalid_argument("traffic không hợp lệ: '" + name +
                                "' (uniform | transpose1 | transpose2)");
}

} // namespace noc
