// Channel pressure ρ (mục 4.3 của bài báo) và routing pressure φ = max ρ (mục 4.5).
#pragma once

#include "routing.h"
#include "topology.h"
#include "traffic.h"

#include <array>
#include <ostream>
#include <string>
#include <vector>

namespace noc
{

enum class Method
{
    Path, // kênh có k trên p đường của một cặp nhận weight × k/p
    Hop,  // kênh nhận weight × Σ hopProb của các đường qua nó (Fig. 9)
};

inline constexpr std::array<Method, 2> kAllMethods{Method::Path, Method::Hop};

const char* MethodName(Method m);
// Ném std::invalid_argument nếu tên không hợp lệ.
Method ParseMethod(const std::string& name);

// ρ của mọi kênh (chỉ số theo Mesh::Channels()) theo cả hai phương pháp.
struct ChannelPressure
{
    std::vector<double> path;
    std::vector<double> hop;

    const std::vector<double>& Get(Method m) const { return m == Method::Path ? path : hop; }
};

// Một lần duyệt cây tìm kiếm cho mỗi luồng, tính đồng thời cả hai phương pháp.
ChannelPressure ComputePressure(Algo algo, const Mesh& mesh, const std::vector<Flow>& flows);

constexpr double kTieEps = 1e-9;

struct PhiResult
{
    double phi;                   // max ρ
    std::vector<int> maxChannels; // mọi kênh có ρ ≥ φ − kTieEps
};

PhiResult MaxPressure(const std::vector<double>& rho);

// PIR tối đa không nghẽn (công thức 6 của bài báo): frate/len/φ = 0.0625/φ.
double PirMax(double phi);

struct PhiOptions
{
    std::string routing = "all"; // all | XY | YX | NF | NF_WS | OE | FULL
    std::string traffic = "all"; // all | uniform | transpose1 | transpose2
    std::string method = "both"; // both | path | hop
    int w = 7;
    int h = 7;
    bool channels = false; // in thêm ρ từng kênh (dòng CHAN)
};

// --mode=phi: bảng routing × traffic × method gồm φ, kênh cổ chai, PIR_max, kèm dòng RESULT.
void RunPhiMode(const PhiOptions& opt, std::ostream& os);

} // namespace noc
