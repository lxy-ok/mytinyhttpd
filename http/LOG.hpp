#pragma once
//日志类是用来记录程序运行时的相关信息的工具。在软件开发中，日志是一种非常重要的调试和排查问题的手段
#include<iostream>

using std::cout;
using std::endl;

enum LogLevel  //日志级别
{   
	INFO,//信息：程序在正常运行中记录的内容。
	WARNING,//警告：用于记录可能会导致潜在问题的情况，但程序仍然可以继续运行。
	ERROR,//错误： 用于记录导致程序无法正常执行某些操作的错误情况，但程序仍然可以继续运行。
	FATAL//致命错误：用于记录导致程序无法继续执行的严重错误情况，通常会导致程序终止运行
};
//LOG(INFO, "server start to access links");这是其他函数调用日志时所写的函数
// _FILE__ 和 __LINE__ 是预定义的宏(系统定义好的)，用于获取当前源文件的文件名和行号
#define LOG(level, message) Log(#level, message, __FILE__, __LINE__) // 打印日志
//日志级别 日志所在文件 行号 
void Log(const std::string& level,const std::string& message,const std::string& fileName, int line)
{
// 日志格式：
// [level]<file:line>{日志内容}==>time
	/*time_t 是 C/C++ 中的一种数据类型，用于表示时间的值。通常情况下
	time_t 类型被定义为整数类型，用来表示从某个特定的时间点开始经过的秒数*/
	time_t tm = time(nullptr);//当前时间的时间戳，并将其赋值给 tm 变量
	cout << "[" << level << "](" << fileName << ":" << line << ")" << "{" << message << "}==> " << ctime(&tm);
}
