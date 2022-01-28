#include "utils.h"
#define _CRT_SECURE_NO_WARNINGS

namespace MF
{
std::vector<std::string> split(const std::string& text, char sep) {
	std::vector<std::string> tokens;
	std::size_t              start = 0, end = 0;
	while ((end = text.find(sep, start)) != std::string::npos) {
		tokens.push_back(text.substr(start, end - start));
		start = end + 1;
	}
	tokens.push_back(text.substr(start));
	return tokens;
}

std::time_t local_time_now() {
	return std::chrono::system_clock::to_time_t(
			std::chrono::system_clock::now());
}

std::string time2string(std::time_t tt) {
	auto              z   = tt;
	auto              zz  = &z;
	auto              zzz = std::localtime(zz);
	std::tm           tm  = *zzz; //Locale time-zone, usually UTC by default.
	std::stringstream ss;
	ss << std::put_time(&tm, "%F %T %z");
	return ss.str();
}
}
