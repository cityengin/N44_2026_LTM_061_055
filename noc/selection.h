// Selection, tách riêng khỏi routing: chọn MỘT hướng trong tập do routeSet trả về.
#pragma once

#include "routing.h"

#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"

namespace noc
{

int CountDirs(DirSet s);

// Hướng thứ k (từ 0) trong s theo thứ tự cố định E, W, N, S.
// Ném std::out_of_range nếu k nằm ngoài [0, CountDirs(s)).
Dir NthDir(DirSet s, int k);

// Chọn ngẫu nhiên đều trong s (s khác rỗng), dùng biến ngẫu nhiên riêng của router.
Dir SelectUniform(DirSet s, const ns3::Ptr<ns3::UniformRandomVariable>& rv);

} // namespace noc
