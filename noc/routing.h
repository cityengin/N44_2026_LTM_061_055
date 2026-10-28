// Hàm routing duy nhất của dự án: bộ tính φ, CDG và router ns-3 đều gọi routeSet.
// Chỉ định tuyến tối thiểu (minimal). Việc chọn một hướng (selection) nằm ở chỗ khác.
#pragma once

#include "topology.h"

#include <array>
#include <cstdint>
#include <string>

namespace noc
{

enum class Algo
{
    XY,
    YX,
    NF,    // âm = W, N (quy ước chính)
    NF_WS, // âm = W, S (biến thể chẩn đoán, xem mục 4.1 của report/bao-cao.md)
    OE,    // Odd-Even theo Chiu (2000)
    FULL,  // fully adaptive minimal, chỉ dùng để kiểm thử (không deadlock-free)
};

inline constexpr std::array<Algo, 6> kAllAlgos{Algo::XY,
                                               Algo::YX,
                                               Algo::NF,
                                               Algo::NF_WS,
                                               Algo::OE,
                                               Algo::FULL};

// Bitmask hướng: bit i ứng với Dir có giá trị i, tức thứ tự cố định E, W, N, S.
using DirSet = std::uint8_t;

inline constexpr std::array<Dir, 4> kDirOrder{Dir::E, Dir::W, Dir::N, Dir::S};
inline constexpr DirSet kAllLinkDirs = 0x0F;

constexpr DirSet
Bit(Dir d)
{
    return static_cast<DirSet>(1u << static_cast<int>(d));
}

constexpr bool
Has(DirSet s, Dir d)
{
    return (s & Bit(d)) != 0;
}

// Tập hướng ra hợp lệ tại cur cho gói đi từ src tới dst.
// Rỗng khi và chỉ khi cur == dst. Không phụ thuộc hướng đi vào.
// src cần cho OE (điều kiện cur.x == src.x của Chiu).
DirSet routeSet(Algo algo, Coord cur, Coord src, Coord dst);

const char* AlgoName(Algo algo);
// Ném std::invalid_argument nếu tên không hợp lệ.
Algo ParseAlgo(const std::string& name);
// true với biến thể chẩn đoán (NF_WS): output phải ghi variant=diagnostic.
bool IsDiagnostic(Algo algo);
// Nhãn variant= trong output: "diagnostic" (NF_WS), "test_only" (FULL), "" với thuật toán chính.
const char* VariantName(Algo algo);

} // namespace noc
