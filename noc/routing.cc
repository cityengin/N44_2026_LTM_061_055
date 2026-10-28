#include "routing.h"

#include <stdexcept>

namespace noc
{
namespace
{

// ROUTE của Chiu (2000), bản minimal. Cột đánh số từ 0;
// cột chẵn cấm E→N, E→S; cột lẻ cấm N→W, S→W.
DirSet
OddEven(Coord cur, Coord src, Coord dst, DirSet ey)
{
    const int dx = dst.x - cur.x;
    const bool curOdd = cur.x % 2 == 1;
    if (dx == 0)
    {
        return ey;
    }
    if (dx > 0)
    {
        if (ey == 0)
        {
            return Bit(Dir::E);
        }
        DirSet s = 0;
        if (curOdd || cur.x == src.x)
        {
            s |= ey;
        }
        if (dst.x % 2 == 1 || dx != 1)
        {
            s |= Bit(Dir::E);
        }
        return s;
    }
    DirSet s = Bit(Dir::W);
    if (!curOdd)
    {
        s |= ey;
    }
    return s;
}

} // namespace

DirSet
routeSet(Algo algo, Coord cur, Coord src, Coord dst)
{
    const int dx = dst.x - cur.x;
    const int dy = dst.y - cur.y; // dy < 0: cần đi N (y giảm)
    if (dx == 0 && dy == 0)
    {
        return 0;
    }
    const DirSet ex = dx > 0 ? Bit(Dir::E) : (dx < 0 ? Bit(Dir::W) : 0);
    const DirSet ey = dy > 0 ? Bit(Dir::S) : (dy < 0 ? Bit(Dir::N) : 0);

    switch (algo)
    {
    case Algo::XY:
        return ex != 0 ? ex : ey;
    case Algo::YX:
        return ey != 0 ? ey : ex;
    case Algo::NF: {
        // Đi hết các hướng âm (W, N) trước, sau đó mới đi hướng dương (E, S).
        const DirSet neg = (dx < 0 ? Bit(Dir::W) : 0) | (dy < 0 ? Bit(Dir::N) : 0);
        return neg != 0 ? neg : (ex | ey);
    }
    case Algo::NF_WS: {
        const DirSet neg = (dx < 0 ? Bit(Dir::W) : 0) | (dy > 0 ? Bit(Dir::S) : 0);
        return neg != 0 ? neg : (ex | ey);
    }
    case Algo::OE:
        return OddEven(cur, src, dst, ey);
    case Algo::FULL:
        return ex | ey;
    }
    throw std::invalid_argument("routeSet: thuật toán không hợp lệ");
}

const char*
AlgoName(Algo algo)
{
    switch (algo)
    {
    case Algo::XY:
        return "XY";
    case Algo::YX:
        return "YX";
    case Algo::NF:
        return "NF";
    case Algo::NF_WS:
        return "NF_WS";
    case Algo::OE:
        return "OE";
    case Algo::FULL:
        return "FULL";
    }
    return "?";
}

Algo
ParseAlgo(const std::string& name)
{
    for (Algo a : kAllAlgos)
    {
        if (name == AlgoName(a))
        {
            return a;
        }
    }
    throw std::invalid_argument("routing không hợp lệ: '" + name +
                                "' (XY | YX | NF | NF_WS | OE | FULL)");
}

bool
IsDiagnostic(Algo algo)
{
    return algo == Algo::NF_WS;
}

const char*
VariantName(Algo algo)
{
    if (IsDiagnostic(algo))
    {
        return "diagnostic";
    }
    return algo == Algo::FULL ? "test_only" : "";
}

} // namespace noc
