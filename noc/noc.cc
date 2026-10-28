// Tái hiện Routing Pressure (Tang, Lin, Palesi, TPDS 2015) trên ns-3.
// Đây là file .cc duy nhất trong noc/ được phép có hàm main (quy tắc scratch của ns-3).
#include "noc-sim.h"
#include "phi.h"
#include "selftest.h"

#include "ns3/core-module.h"

#include <exception>
#include <iostream>
#include <string>
#include <vector>

using namespace ns3;

int
main(int argc, char* argv[])
{
    // Cờ ẩn (không hiện trong --help): thêm một test HARD fail để kiểm tra exit code.
    // Gỡ khỏi argv trước khi CommandLine parse, vì CommandLine từ chối tham số lạ.
    bool forceFail = false;
    std::vector<char*> args;
    for (int i = 0; i < argc; ++i)
    {
        if (std::string(argv[i]) == "--selftest-force-fail")
        {
            forceFail = true;
        }
        else
        {
            args.push_back(argv[i]);
        }
    }

    std::string mode;
    noc::PhiOptions phi;
    noc::SimConfig sim;

    CommandLine cmd(__FILE__);
    cmd.AddValue("mode", "phi | selftest | sim | simcheck", mode);
    cmd.AddValue("routing", "all | XY | YX | NF | NF_WS | OE | FULL", phi.routing);
    cmd.AddValue("traffic", "all | uniform | transpose1 | transpose2", phi.traffic);
    cmd.AddValue("method", "mode=phi: both | path | hop", phi.method);
    cmd.AddValue("W", "số cột của lưới", phi.w);
    cmd.AddValue("H", "số hàng của lưới", phi.h);
    cmd.AddValue("channels", "in thêm từng kênh (phi: dòng CHAN, sim: dòng SIMCHAN)", phi.channels);
    cmd.AddValue("pir", "mode=sim: gói / node / chu kỳ", sim.pir);
    cmd.AddValue("warmup", "mode=sim: chu kỳ khởi động", sim.warmup);
    cmd.AddValue("measure", "mode=sim: độ dài cửa sổ đo (chu kỳ)", sim.measure);
    cmd.AddValue("drain", "mode=sim: thời gian drain tối đa (chu kỳ)", sim.drain);
    cmd.Parse(static_cast<int>(args.size()), args.data());

    try
    {
        if (mode == "selftest")
        {
            return noc::RunSelfTest(forceFail) == 0 ? 0 : 1;
        }
        if (mode == "phi")
        {
            noc::RunPhiMode(phi, std::cout);
            return 0;
        }
        if (mode == "sim")
        {
            if (phi.routing == "all" || phi.traffic == "all")
            {
                std::cerr << "mode=sim cần --routing và --traffic cụ thể\n";
                return 2;
            }
            sim.algo = noc::ParseAlgo(phi.routing);
            sim.pattern = noc::ParsePattern(phi.traffic);
            sim.w = phi.w;
            sim.h = phi.h;
            sim.channels = phi.channels;
            noc::RunSimMode(sim, std::cout);
            return 0;
        }
        if (mode == "simcheck")
        {
            return noc::RunSimCheck(std::cout) == 0 ? 0 : 1;
        }
        std::cerr << "Thiếu hoặc sai --mode (phi | selftest | sim | simcheck): '" << mode << "'\n";
        return 2;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Lỗi: " << e.what() << "\n";
        return 2;
    }
}
