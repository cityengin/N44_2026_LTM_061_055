// Channel dependency graph (Dally & Seitz): có cạnh ch1→ch2 khi một đường hợp lệ nào đó
// (của mọi cặp src≠dst) đi qua ch1 rồi ngay sau đó ch2, kể cả đi thẳng.
// CDG không có chu trình ⇒ routing không deadlock.
#pragma once

#include "routing.h"
#include "topology.h"

#include <string>
#include <vector>

namespace noc
{

struct CdgResult
{
    int channels;
    long edges;            // tổng số phụ thuộc (không trùng)
    long turnEdges;        // trong đó số phụ thuộc có đổi hướng
    bool acyclic;
    std::vector<int> cycle; // một chu trình mẫu (các kênh theo thứ tự), rỗng nếu acyclic
};

// Phát hiện chu trình bằng DFS 3 màu.
CdgResult CheckCdg(Algo algo, const Mesh& mesh);

// "0-1>1-4>4-3>3-0": các kênh của chu trình, kênh cuối nối về kênh đầu.
std::string CycleString(const Mesh& mesh, const std::vector<int>& cycle);

} // namespace noc
