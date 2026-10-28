#include "topology.h"

#include <cstdlib>
#include <stdexcept>

namespace noc
{

const char*
DirName(Dir d)
{
    switch (d)
    {
    case Dir::N:
        return "N";
    case Dir::E:
        return "E";
    case Dir::S:
        return "S";
    case Dir::W:
        return "W";
    case Dir::Local:
        return "L";
    }
    return "?";
}

Dir
Opposite(Dir d)
{
    switch (d)
    {
    case Dir::N:
        return Dir::S;
    case Dir::E:
        return Dir::W;
    case Dir::S:
        return Dir::N;
    case Dir::W:
        return Dir::E;
    case Dir::Local:
        return Dir::Local;
    }
    throw std::invalid_argument("Opposite: hướng không hợp lệ");
}

Mesh::Mesh(int w, int h)
    : m_w(w),
      m_h(h)
{
    if (w < 1 || h < 1)
    {
        throw std::invalid_argument("Mesh: W, H phải >= 1");
    }
    m_chanIndex.assign(NumNodes() * kNumLinkDirs, -1);
    for (int u = 0; u < NumNodes(); ++u)
    {
        for (int k = 0; k < kNumLinkDirs; ++k)
        {
            Dir d = static_cast<Dir>(k);
            int v = Neighbor(u, d);
            if (v >= 0)
            {
                m_chanIndex[u * kNumLinkDirs + k] = NumChannels();
                m_channels.push_back({u, v, d});
            }
        }
    }
}

bool
Mesh::Contains(int x, int y) const
{
    return x >= 0 && x < m_w && y >= 0 && y < m_h;
}

Coord
Mesh::ToCoord(int id) const
{
    if (id < 0 || id >= NumNodes())
    {
        throw std::out_of_range("Mesh::ToCoord: id ngoài lưới");
    }
    return {id % m_w, id / m_w};
}

int
Mesh::ToId(int x, int y) const
{
    if (!Contains(x, y))
    {
        throw std::out_of_range("Mesh::ToId: tọa độ ngoài lưới");
    }
    return y * m_w + x;
}

int
Mesh::Distance(int a, int b) const
{
    Coord ca = ToCoord(a);
    Coord cb = ToCoord(b);
    return std::abs(ca.x - cb.x) + std::abs(ca.y - cb.y);
}

int
Mesh::Neighbor(int id, Dir d) const
{
    Coord c = ToCoord(id);
    switch (d)
    {
    case Dir::N:
        c.y -= 1;
        break;
    case Dir::E:
        c.x += 1;
        break;
    case Dir::S:
        c.y += 1;
        break;
    case Dir::W:
        c.x -= 1;
        break;
    case Dir::Local:
        return -1;
    }
    return Contains(c.x, c.y) ? ToId(c.x, c.y) : -1;
}

const Channel&
Mesh::GetChannel(int ch) const
{
    return m_channels.at(ch);
}

int
Mesh::ChannelId(int u, Dir d) const
{
    if (d == Dir::Local)
    {
        return -1;
    }
    ToCoord(u); // kiểm tra u hợp lệ
    return m_chanIndex[u * kNumLinkDirs + static_cast<int>(d)];
}

std::string
Mesh::ChannelName(int ch) const
{
    const Channel& c = GetChannel(ch);
    return std::to_string(c.u) + "-" + std::to_string(c.v);
}

} // namespace noc
