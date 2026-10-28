// Lưới W×H: gốc góc trên-trái, x tăng về Đông, y tăng về Nam, id = y*W + x.
// Kênh = link có hướng u→v giữa hai node kề nhau.
#pragma once

#include <string>
#include <vector>

namespace noc
{

// Thứ tự cố định E, W, N, S: giá trị của Dir cũng là vị trí bit trong DirSet (routing.h).
enum class Dir : int
{
    E = 0, // x tăng
    W = 1, // x giảm
    N = 2, // y giảm
    S = 3, // y tăng
    Local = 4,
};

constexpr int kNumLinkDirs = 4; // E, W, N, S

const char* DirName(Dir d);
Dir Opposite(Dir d);

struct Coord
{
    int x;
    int y;
};

struct Channel
{
    int u;
    int v;
    Dir dir; // hướng đi ra từ u
};

class Mesh
{
  public:
    Mesh(int w, int h);

    int W() const { return m_w; }
    int H() const { return m_h; }
    int NumNodes() const { return m_w * m_h; }

    bool Contains(int x, int y) const;
    Coord ToCoord(int id) const;
    int ToId(int x, int y) const;
    int Distance(int a, int b) const; // Manhattan

    // −1 nếu đi ra ngoài biên.
    int Neighbor(int id, Dir d) const;

    int NumChannels() const { return static_cast<int>(m_channels.size()); }
    const std::vector<Channel>& Channels() const { return m_channels; }
    const Channel& GetChannel(int ch) const;
    // −1 nếu không có link theo hướng d.
    int ChannelId(int u, Dir d) const;
    // Dạng "u-v" như bài báo, ví dụ "17-24".
    std::string ChannelName(int ch) const;

  private:
    int m_w;
    int m_h;
    std::vector<Channel> m_channels;
    std::vector<int> m_chanIndex; // u*kNumLinkDirs + dir → id kênh hoặc −1
};

} // namespace noc
