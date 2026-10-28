// Mô phỏng NoC trên ns-3 (quy ước mô phỏng: mục 3.3 của report/bao-cao.md).
#pragma once

#include "routing.h"
#include "traffic.h"

#include <cstdint>
#include <limits>
#include <ostream>
#include <string>
#include <vector>

namespace noc
{

constexpr int kPacketFlits = 8;      // len
constexpr int kCyclesPerPacket = 16; // len / frate: thời gian một gói chiếm link

struct Injection
{
    int64_t timeNs;
    int src;
    int dst;
};

struct SimConfig
{
    Algo algo = Algo::XY;
    Pattern pattern = Pattern::Transpose1;
    int w = 7;
    int h = 7;
    double pir = 0.01;       // gói / node / chu kỳ
    int64_t warmup = 1000;   // chu kỳ (= ns)
    int64_t measure = 20000; // cửa sổ đo [warmup, warmup+measure)
    int64_t drain = 20000;   // thời gian tối đa sau cửa sổ đo
    bool channels = false;   // in thêm dòng SIMCHAN
    // Nếu khác rỗng: chỉ phát đúng các gói này (dùng cho test), không dùng bộ tạo traffic.
    std::vector<Injection> injections;
};

struct SimResult
{
    uint32_t seed = 0;
    uint64_t run = 0;
    long generated = 0;       // gói tạo trong cửa sổ đo
    long delivered = 0;       // trong số đó, đã tới đích khi dừng
    long undelivered = 0;
    bool saturated = false;   // undelivered > 0
    long drops = 0;           // Drop của queue + PhyRxDrop
    long hopMismatch = 0;     // gói tới đích (bất kỳ) có hops ≠ khoảng cách Manhattan
    long arrivedInWindow = 0; // gói (bất kỳ) tới đích trong cửa sổ đo
    double latAvg = 0.0;      // chu kỳ, gói của cửa sổ đã tới đích
    double latAvgLb = 0.0;    // tính cả gói chưa tới: thời điểm dừng − thời điểm tạo
    double latP95 = 0.0;
    double hopsAvg = 0.0;
    // min/max của (lat_i − 16·hops_i) trên các gói của cửa sổ đã tới đích; âm nghĩa là có bug.
    double slackMin = std::numeric_limits<double>::quiet_NaN();
    double slackMax = std::numeric_limits<double>::quiet_NaN();
    std::vector<long> hopsHist; // số gói của cửa sổ đã tới đích theo số hop
    double throughput = 0.0;  // flit tới đích trong cửa sổ / (W·H·measure)
    double offered = 0.0;     // flit tạo trong cửa sổ / (W·H·measure)
    int64_t stopNs = 0;
    uint32_t minFrame = 0;
    uint32_t maxFrame = 0;
    std::vector<long> chanTx;       // số gói bắt đầu truyền trên từng kênh trong cửa sổ đo
    std::vector<int64_t> txStartNs; // thời điểm PhyTxBegin của tối đa 64 lần truyền đầu tiên
    // U_đo lớn nhất = max_ch chanTx·16/measure (≤ 1 khi measure chia hết cho 16) và kênh đạt
    // max (kênh có chỉ số nhỏ nhất nếu hòa); uMaxCh = −1 nếu không có kênh nào.
    double uMax = 0.0;
    int uMaxCh = -1;

    // Dự đoán từ bộ tính φ (cùng routeSet), đã chuẩn hóa: uniform chia N−1.
    std::vector<double> rhoHop;
    std::vector<double> rhoPath;
    double phiHop = 0.0;
};

SimResult RunSim(const SimConfig& cfg);

// Phân vị theo nearest-rank: phần tử thứ ⌈q·n⌉ (từ 1) của v đã sắp xếp; NaN nếu v rỗng.
double PercentileNearestRank(std::vector<int64_t> v, double q);

// Dòng RESULT,mode=sim,... (không có xuống dòng).
std::string FormatResult(const SimConfig& cfg, const SimResult& r);

// --mode=sim: chạy một lần, in dòng RESULT và (nếu cfg.channels) các dòng SIMCHAN.
void RunSimMode(const SimConfig& cfg, std::ostream& os);

// --mode=simcheck: các kiểm tra chạy lâu hơn, không thuộc selftest. Trả về số kiểm tra FAIL.
//  - tải thấp: XY, transpose1/transpose2, PIR 0.001: lat_avg ≈ 16·(khoảng cách TB),
//    throughput ≈ PIR·8·flows/(W·H), drops = 0;
//  - NF/transpose1 so với NF_WS/transpose2 (ảnh gương qua R) trên 5 seed, α = 0.05.
int RunSimCheck(std::ostream& os);

} // namespace noc
