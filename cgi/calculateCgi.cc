#include "CgiCommon.h"

int main()
{
    std::string query_string;
    GetQueryString(GetMethod(), query_string); //获取参数

    //以&为分隔符将两个操作数分开
    std::string str1;
    std::string str2;
    CutString(query_string, "&", str1, str2);

    //以=为分隔符分别获取两个操作数的值
    std::string name1;
    std::string value1;
    CutString(str1, "=", name1, value1);
    std::string name2;
    std::string value2;
    CutString(str2, "=", name2, value2);

    //处理数据
    int x = atoi(value1.c_str());
    int y = atoi(value2.c_str());
    std::cout<<"<html>";
    std::cout<<"<head><meta charset=\"UTF-8\"></head>";
    std::cout<<"<body>";
    std::cout<<"<h3>"<<x<<" + "<<y<<" = "<<x+y<<"</h3>";
    std::cout<<"<h3>"<<x<<" - "<<y<<" = "<<x-y<<"</h3>";
    std::cout<<"<h3>"<<x<<" * "<<y<<" = "<<x*y<<"</h3>";
    if(x % y)
    {
      double dx = x;
      double dy = y;
      std::cout<<"<h3>"<<x<<" / "<<y<<" = "<<dx / dy<<"</h3>"; //除0后cgi程序崩溃，属于异常退出
    }
    else 
    {
      std::cout<<"<h3>"<<x<<" / "<<y<<" = "<<x/y<<"</h3>"; //除0后cgi程序崩溃，属于异常退出
    }
    std::cout<<"</body>";
    std::cout<<"</html>";

    return 0;
}
