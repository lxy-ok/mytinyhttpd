// cgi程序中0、1重定向到了匿名管道，0重定向为output[0]，1重定向为input[1]，故cin就是从管道读，cout就是向管道写

// 对于CGI程序来说，其不用关心中间HTTP通信细节，只需要处理特定业务逻辑即可。
// 那么在CGI的角度，其完全可以忽略中间HTTP通信的过程，重定向了0和1，此时其标准输入和标准输出就是浏览器，只不过是数据的传输路径变长了。

#include"CgiCommon.h"

int main()
{
  cerr << "=====================cgi running" << endl;  
  
  char* method = GetMethod();

  std::string queryString;
  GetQueryString(method, queryString); // 获取参数

  // 这里就简单将参数分离一下，一个参数的格式的例子；a=100&b=200
    // 先切&两边
  std::string left, right;
  CutString(queryString, "&", left, right);

  std::string leftName, leftValue;
  CutString(left, "=", leftName, leftValue);

  std::string rightName, rightValue;
  CutString(right, "=", rightName, rightValue);

  cout << leftName << "=" << leftValue << endl;
  cout << rightName << "=" << rightValue << endl;

  cerr << leftName << "=" << leftValue << endl;
  cerr << rightName << "=" << rightValue << endl;

  cerr << "=====================cgi end" << endl;
  return 0;
}
