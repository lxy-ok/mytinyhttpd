#pragma once

#include<iostream>
using std::cout;
using std::cerr;
using std::endl;

#include<unistd.h>

const static size_t INFO = 1;
const static size_t ERROR = 2;

#define CGILOG(level, str) CgiLog(#level, str)

void CgiLog(const std::string& level, const std::string& message)
{
  cerr << "CGI " << level << "==>" << message << endl;
}

// 获取请求方法
char* GetMethod()
{
  char* method = getenv("METHOD"); // 获取请求方法，用来判断如何获取参数
  if(method == nullptr)
  {
    CGILOG(ERROR, "get method env failed");
    exit(2);
  }
  CGILOG(INFO, "get method:" + std::string(method));

  return method;
}

// 获取参数
void GetQueryString(const std::string &method, std::string& queryString)
{
  CGILOG(INFO, "GetQueryString function start");
  // 分两种，一种POST的，一种带参GET的
  if(method == "GET")
  { // 从环境变量中获得参数
    CGILOG(INFO, "method is GET");
    queryString = getenv("QUERY_STRING");
  }
  else if(method == "POST")
  { // 从管道(0)中获取参数，可以直接用cin，但是可能会有回车啥的，所以就用read
    CGILOG(INFO, "method is POST");
    size_t contentLength = std::stoul(getenv("CONTENT_LENGTH"));
    char ch;
    while(contentLength--)
    {
      read(0, &ch, 1); // ---------------------------这里未处理read返回值
      queryString.push_back(ch);
    }
  }
  else
  {
    CGILOG(ERROR, "unknow method[" + method + ']');
    exit(3);
  }

  CGILOG(INFO, "get query string:" + queryString);
}

// 切分函数
bool CutString(const std::string& str, const std::string& sep, std::string& leftStr, std::string& rightStr)
{
  size_t pos = str.find(sep);
  if(pos != std::string::npos)
  {
    leftStr = str.substr(0, pos);
    rightStr = str.substr(pos + sep.size());
  
    return true;
  }
  else
  {
    return false;
  }
}