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

std::vector<std::string> split(const std::string& text, char sep); //ʵ���ַ����ָstring.split()
std::time_t              local_time_now();                         //�õ���ǰʱ��
std::string              time2string(std::time_t tt);              //��ʱ��תΪ�ַ���
bool                     is_number(const std::string& s);          //�ж�һ���ַ����Ƿ�������
}
