#include "selection.h"

#include <bit>
#include <stdexcept>

namespace noc
{

int
CountDirs(DirSet s)
{
    return std::popcount(static_cast<unsigned>(s & kAllLinkDirs));
}

Dir
NthDir(DirSet s, int k)
{
    if (k >= 0)
    {
        for (Dir d : kDirOrder)
        {
            if (Has(s, d) && k-- == 0)
            {
                return d;
            }
        }
    }
    throw std::out_of_range("NthDir: k nằm ngoài tập hướng");
}

Dir
SelectUniform(DirSet s, const ns3::Ptr<ns3::UniformRandomVariable>& rv)
{
    const int n = CountDirs(s);
    if (n == 0)
    {
        throw std::logic_error("SelectUniform: tập hướng rỗng");
    }
    return NthDir(s, static_cast<int>(rv->GetInteger(0, n - 1)));
}

} // namespace noc
