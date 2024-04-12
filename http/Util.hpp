#include "Common.h"

class Util
{
public:
  static bool CutString(const std::string &str, std::string &leftOut, std::string &rightOut, const std::string &sep)
  {
    size_t pos = str.find(sep);
    if(pos != std::string::npos)
    { // 可以分割
      leftOut = str.substr(0, pos);
      rightOut = str.substr(pos + sep.size());
      return true;
    }
    return false;
  }


  // 读取单行的时候可能是\r\n结尾，也可能是\r结尾,还可能是\n结尾，统一都按照以\n结尾来处理。
  static int ReadLine(int sock, std::string &out)
  {
    // ch只要初始不是\r、\n就行
    char ch = 'X';
    while (ch != '\n')
    { // 统一设置为以\n结尾
      ssize_t s = recv(sock, &ch, 1, 0);
      if(s > 0)
      {
        if (ch == '\r')
        {                               // 查看下一位是啥
          recv(sock, &ch, 1, MSG_PEEK); // 窥探下一位是啥
          if (ch == '\n')
          {                             // 如果是\n就直接读走，后面正好直接push_back
            recv(sock, &ch, 1, 0); // 直接读走，把ch中的\r覆盖为\n
          }
          else
          {            // 不是\n，表示\r后面是有用数据，不能读走
            ch = '\n'; // 直接将ch中的\r覆盖为\n，这样就在push_back的时候只会有\n
          }
        }
        // ch走到这里就只有两种情况：非\r、\n的普通字符 和 \n
        out.push_back(ch); // 放到其中就ok
      }
      else if(s == 0)
      {
        return 0;
      }
      else
      {
        return -1;
      }

    }
    return out.size();
  }
};
