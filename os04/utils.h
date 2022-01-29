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

std::vector<std::string> split(const std::string& text, char sep); //实现字符串分割，string.split()
std::time_t              local_time_now();                         //得到当前时间
std::string              time2string(std::time_t tt);              //把时间转为字符串
bool                     is_number(const std::string& s);          //判断一个字符串是否是整数
}
