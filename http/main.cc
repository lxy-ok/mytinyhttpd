#include"HttpServer.hpp"

#include<memory>

// 用法
void Usage(std::string s)
{
  cout << "Usage:\n\t" << s << " port" << endl;
}//告诉你正确的输入格式
/*argc表示命令行参数的数量，包括程序名称本身。
argv[],argv[0] 通常是程序的名称，而后续元素则是命令行传递给程序的参数。
例如：./my_program 8081 表示在命令行中运行名为 my_program 的可执行文件
并且向该程序传递了一个参数 8081。
在程序内部，argc 的值将为 2，而 argv[0] 将指向程序的名称 "./my_program"，
而 argv[1] 则将指向字符串 "8081"。程序可以根据这个参数来执行相应的操作*/
int main(int argc, char* argv[])
{
  if(argc != 2)
  {
    Usage(argv[0]);
    exit(4);//异常退出
  }
//命令行参数是以字符串形式传递给程序的，atoi将端口号转换为数字
  std::shared_ptr<HttpServer> hsvr(new HttpServer(atoi(argv[1])));
  hsvr->Loop();
   

  return 0;
}
