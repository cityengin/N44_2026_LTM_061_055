// Tái hiện Routing Pressure (Tang, Lin, Palesi, TPDS 2015) trên ns-3.
// Đây là file .cc duy nhất trong noc/ được phép có hàm main (quy tắc scratch của ns-3).
#include "selftest.h"

#include "ns3/core-module.h"

#include <exception>
#include <iostream>
#include <string>

using namespace ns3;

int
main(int argc, char* argv[])
{
    std::string mode;

    CommandLine cmd(__FILE__);
    cmd.AddValue("mode", "phi | selftest | sim", mode);
    cmd.Parse(argc, argv);

    try
    {
        if (mode == "selftest")
        {
            return noc::RunSelfTest() == 0 ? 0 : 1;
        }
        if (mode == "phi" || mode == "sim")
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
