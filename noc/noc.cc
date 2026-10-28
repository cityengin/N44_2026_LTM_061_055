// Tái hiện Routing Pressure (Tang, Lin, Palesi, TPDS 2015) trên ns-3.
// Đây là file .cc duy nhất trong noc/ được phép có hàm main (quy tắc scratch của ns-3).
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

    CommandLine cmd(__FILE__);
    cmd.AddValue("mode", "phi | selftest | sim", mode);
    cmd.AddValue("routing", "all | XY | YX | NF | NF_WS | OE | FULL", phi.routing);
    cmd.AddValue("traffic", "all | uniform | transpose1 | transpose2", phi.traffic);
    cmd.AddValue("method", "both | path | hop", phi.method);
    cmd.AddValue("W", "số cột của lưới", phi.w);
    cmd.AddValue("H", "số hàng của lưới", phi.h);
    cmd.AddValue("channels", "mode=phi: in thêm ρ từng kênh (dòng CHAN)", phi.channels);
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
            std::cerr << "mode=" << mode << " chưa cài đặt\n";
            return 2;
        }
        std::cerr << "Thiếu hoặc sai --mode (phi | selftest | sim): '" << mode << "'\n";
        return 2;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Lỗi: " << e.what() << "\n";
        return 2;
    }
}
