// Mẫu lưu lượng. Mỗi luồng có trọng số 1; node tự map vào chính nó không phát gói.
#pragma once

#include "topology.h"

#include <array>
#include <string>
#include <vector>

namespace noc
{

enum class Pattern
{
    Uniform,
    Transpose1, // (x,y) → (N−1−y, N−1−x)
    Transpose2, // (x,y) → (y, x)
};

inline constexpr std::array<Pattern, 3> kAllPatterns{Pattern::Uniform,
                                                     Pattern::Transpose1,
                                                     Pattern::Transpose2};

struct Flow
{
    int src;
    int dst;
    double weight;
};

// uniform: mọi cặp src≠dst. Transpose cần lưới vuông (ném std::invalid_argument nếu W≠H).
std::vector<Flow> MakeFlows(Pattern p, const Mesh& m);

// Hệ số chuẩn hóa khi suy PIR_max: uniform 1/(N−1) (mỗi node chia đều cho N−1 đích), còn lại 1.
double NormFactor(Pattern p, const Mesh& m);

const char* PatternName(Pattern p);
// Ném std::invalid_argument nếu tên không hợp lệ.
Pattern ParsePattern(const std::string& name);

} // namespace noc
