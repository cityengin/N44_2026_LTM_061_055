// Định dạng số cho các dòng RESULT / CHAN / SIMCHAN.
#pragma once

#include <iomanip>
#include <sstream>
#include <string>

namespace noc
{

inline std::string
Fixed(double v, int digits = 6)
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(digits) << v;
    return ss.str();
}

inline std::string
Short(double v)
{
    std::ostringstream ss;
    ss << v;
    return ss.str();
}

} // namespace noc
