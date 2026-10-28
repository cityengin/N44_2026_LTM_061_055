// Mô phỏng NoC trên ns-3 (quy ước: mục "Mô phỏng" trong CLAUDE.md).
#pragma once

#include "routing.h"
#include "traffic.h"

#include <cstdint>
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
    double throughput = 0.0;  // flit tới đích trong cửa sổ / (W·H·measure)
    double offered = 0.0;     // flit tạo trong cửa sổ / (W·H·measure)
    int64_t stopNs = 0;
    uint32_t minFrame = 0;
    uint32_t maxFrame = 0;
    std::vector<long> chanTx;       // số gói bắt đầu truyền trên từng kênh trong cửa sổ đo
    std::vector<int64_t> txStartNs; // thời điểm PhyTxBegin của tối đa 64 lần truyền đầu tiên

    // Dự đoán từ bộ tính φ (cùng routeSet), đã chuẩn hóa: uniform chia N−1.
    std::vector<double> rhoHop;
    std::vector<double> rhoPath;
    double phiHop = 0.0;
};

SimResult RunSim(const SimConfig& cfg);

// Dòng RESULT,mode=sim,... (không có xuống dòng).
std::string FormatResult(const SimConfig& cfg, const SimResult& r);

// --mode=sim: chạy một lần, in dòng RESULT và (nếu cfg.channels) các dòng SIMCHAN.
void RunSimMode(const SimConfig& cfg, std::ostream& os);

} // namespace noc
