// Cây tìm kiếm (Fig. 8 của bài báo): gốc là src, mỗi lá là dst, mỗi nhánh từ gốc tới lá
// là một đường minimal mà routeSet cho phép. Bộ tính φ và CDG dùng chung cách duyệt này.
#pragma once

#include "routing.h"
#include "topology.h"

#include <functional>
#include <vector>

namespace noc
{

// nodes = src, ..., dst. hopProb = tích các 1/|routeSet| tại mỗi node trên đường,
// tức trọng số nhánh như Fig. 9 (tương đương router chọn hướng ngẫu nhiên đều).
using PathFn = std::function<void(const std::vector<int>& nodes, double hopProb)>;

// Gọi fn cho từng đường từ src tới dst, theo thứ tự hướng E, W, N, S tại mỗi node.
// Ném std::logic_error nếu routeSet trả tập rỗng trước khi tới dst, hoặc trả hướng
// đi ra ngoài lưới hay không giảm khoảng cách tới dst.
void ForEachPath(Algo algo, const Mesh& mesh, int src, int dst, const PathFn& fn);

long CountPaths(Algo algo, const Mesh& mesh, int src, int dst);

} // namespace noc
