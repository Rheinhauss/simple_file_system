#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace MF
{
//using std::vector;
//using std::string;

std::vector<std::string> split(const std::string& text, char sep);
std::time_t              local_time_now();
std::string              time2string(std::time_t tt);
}
