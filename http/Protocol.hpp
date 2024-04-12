#pragma once
#include"Common.h"
#include"Util.hpp"
/*enum 枚举类型名 {
    枚举值1,
    枚举值2,
    ...
};
*/
/*---------------状态码---------------*/
enum StatusCode
{
  OK = 200, // 正常情况
  BAD_REQUEST = 400, // 客户端发送的请求有语法错误
  NOT_FOUND = 404, // 找不到页面
  SERVER_ERROR = 500 // 服务器内部错误
};

const static char *WEB_ROOT = "wwwroot"; // Web根目录
const static char *HOME_PAGE = "index.html"; // 主页文件名
const static char *END_OF_LINE = "\r\n"; // 每行的\r\n

const static char *BAD_REQUEST_PAGE = "400.html"; // 网页丢失页面
const static char *NOT_FOUND_PAGE = "404.html"; // 网页丢失页面
const static char *SERVER_ERROR_PAGE = "500.html"; // 网页丢失页面

// HttpResponse状态码转状态码描述
static std::string StatusCodeToDesc(size_t statusCode)
{
  static std::unordered_map<std::size_t, std::string> statusDescMap = 
  {
    {200, "OK"},
    {400, "BAD_REQUEST"},
    {404, "NOT_FOUND"},
    {500, "SERVER_ERROR"},
  };

  if(statusDescMap.count(statusCode))
    return statusDescMap[statusCode];

  // std::string res;
  // switch (statusCode)
  // {
  // case 200:
  //   res = "OK";
  //   break;
  // case 400:
  //   res = "BAD_REQUEST";
  //   break;
  // case 404:
  //   res = "NOT_FOUND";
  //   break;
  // case 500:
  //   res = "SERVER_ERROR";
  //   break;
  // default:
  //   break;
  // }
  // return res;
}

// 文件后缀转HttpResponse中的Content-Type
static std::string SuffixToDesc(const std::string &suffix)
{
  static std::unordered_map<std::string, std::string> suffixMap = 
  {
    {"html", "text/html"},
    {"css", "text/css"},
    {"js", "application/javascript"},
    {"jpg", "application/x-jpg"},
    {"xml", "application/xml"}
  };

  return suffixMap[suffix];
}

// 请求报文的相关字段
class HttpRequest
{
public:
  std::string _requestLine; // 请求行
    bool _cgi; // POST、带参GET、请求资源为可执行程序，用cgi解决
  std::string _method; // 请求方法 GET或者POST
  std::string _uri; // url 如/a/b/c/tmp.html
    std::string _path; // uri中的路径
      size_t _size; // 非CGI模式响应正文的大小，也就是_path对应文件大小
      std::string _suffix; // _path文件对应后缀
    std::string _queryString; // GET方法中如果uri带参了会用到
  std::string _version; // 版本 如http/1.1

public:
  std::vector<std::string> _requestHeader; // 请求报头，多行，都放到vector中
  std::unordered_map<std::string, std::string> _headKV; // 以KV形式存放所有的请求报头
  size_t _contentLenth; // 请求中的正文长度

public:
  std::string _blank; // 空行
  std::string _requestBody; // 请求正文

  HttpRequest()
    : _cgi(false)
    , _size(0)
    , _contentLenth(0)
  {}
};

// 响应报文的相关字段
class HttpResponse
{
public:
  std::string _statusLine; // 状态行
  std::vector<std::string> _responseHeader; // 响应报头
  std::string _blank; // 空行
  std::string _responseBody; // 响应正文

  size_t _statusCode;
  int _fd; // 处理非CGI的时候要打开的文件，即调用sendfile要打开的文件
public:
  HttpResponse()
    : _blank(END_OF_LINE)
    , _statusCode(OK)
    , _fd(-1)
  {}

  ~HttpResponse()
  {
    if(_fd >= 0) close(_fd); // 必须关，不然文件描述符泄漏（普通文件）
  }
};

// 端点，就是服务端，该类中包含服务端要做的事情
class EndPoint
{
  // 服务端，要做三件事：
  // 1. 接收请求并分析请求
  // 2. 构建响应
  // 3. 返回响应
private:
  int _sock; // 用来通信的sock
  HttpRequest _httpRequest; // 对端发来的请求
  HttpResponse _httpResponse; // 当前端的响应

  bool stop;
public:
  EndPoint(int sock)
    : _sock(sock)
    , stop(false)
  {}

  ~EndPoint()
  {
    // if(_sock >= 0) close(_sock);
    // 这里不再需要让EndPoint的析构来close(_sock)了，因为线程池中线程执行完毕任务之后会关掉
  }

private: // 接收请求内部使用的接口
  // 接收请求行
  void RecvRequestLine()
  {
    assert(_sock >= 0);

    if(Util::ReadLine(_sock, _httpRequest._requestLine) > 0)
    {
      _httpRequest._requestLine.pop_back();
      LOG(INFO, "recv request line ::" + _httpRequest._requestLine);
    }
    else
    { // 读取失败，直接让本次通信停止
      stop = true;
      LOG(ERROR, "RecvRequestLine failed");
      return;
    }
  }

  // 解析请求行，把 请求方法 uri找哪一个页面，后续的页面渲染等 协议版本 搞出来
  void parseRequestLine()
  {
    std::stringstream s(_httpRequest._requestLine);
    s >> _httpRequest._method >> _httpRequest._uri >> _httpRequest._version;

     // 可能读取到的_method不是大写的，需要手动转为大写
    std::transform(_httpRequest._method.begin(), _httpRequest._method.end(), _httpRequest._method.begin(), ::toupper);

        // 测试请求行解析对不对
    // cout << "------------------------------" << endl;
    // cout << _httpRequest._requestLine << endl;
    // cout << _httpRequest._method << "==" << _httpRequest._uri << "==" << _httpRequest._version << endl;
    // cout << "------------------------------" << endl;
  }

  // 接收请求报头
  void RecvRequestHeader()
  { // 请求报头中有很多行，都按行读取到vector中
    std::vector<std::string> &header = _httpRequest._requestHeader;
    std::string tmp;
    while(true)
    {
      int length = Util::ReadLine(_sock, tmp);
      if(length <= 0)
      {
        stop = true;
        LOG(ERROR, "RecvRequestHeader failed");
        return;
      }
      if(length == 1)
      { // 只有一个\n，就是空行，直接放到_blank中
        _httpRequest._blank = tmp;
        break;
      }

      // 不是空行，每个都放到vetor
      tmp.pop_back();
      header.push_back(tmp);
      tmp.clear();
    }
  }

  // 请求报头中每一行都是K-V的，所以这里也直接保存成KV的，保存到unoreded_map中
  void ParseRequestHeader()
  {
    std::string key;
    std::string value;
    for(auto &str : _httpRequest._requestHeader)  // 多行请求报头接收到了vector中
    {
        // 请求报头中的KV是以 冒号+空格 分隔的
      if(Util::CutString(str, key, value, ": "))
      {
        _httpRequest._headKV.insert({key, value});
      }
      else
      {
        LOG(ERROR, "_httpRequest._requestHeader CutString failed");
      }
    }
  }

  bool IsNeedToRecvRequestBody()
  { //只考虑请求方法是POST时才读取正文
    if(_httpRequest._method == "POST")
    { // 请求方法若是POST，那报文中一定会有一个Content-Length字段，根据这个K找到报文长度即可
      _httpRequest._contentLenth = std::stoi(_httpRequest._headKV["Content-Length"]);
      if(_httpRequest._contentLenth != 0)
      {
        LOG(INFO, "POST method, recv Content-Length: " + _httpRequest._headKV["Content-Length"]);
        return true;
      }

      LOG(WARNING, "POST method but recv Content-Lenth is 0");
    }

    return false;
  }

  // 接收请求正文
  void RecvRequestBody()
  { // 如果请求方法_method是GET，那就一定没有请求正文，如果是POST那就会有请求正文
    if(IsNeedToRecvRequestBody())
    {
      std::string& body = _httpRequest._requestBody;
      char ch = 0;
      int contentLength = _httpRequest._contentLenth;
      while(contentLength--)
      {
        ssize_t s = recv(_sock, &ch, 1, 0);
        if(s > 0)
        {
          body.push_back(ch);
        }
        else  // 读取正文对端写关闭或者读取失败-------------------------------------
        {
          LOG(ERROR, "RecvRequestBody failed");
          stop = true;
          return;
        }
      }
    }
  }
public:
  bool RecvRequest() // 接收请求并分析请求
  {
    bool end = false;
    RecvRequestLine();// 接收请求行
    if(stop == false)
    { // 请求行接收未出错就继续
      RecvRequestHeader();// 接收请求报头
      if(stop == false)
      { // 请求报头读取没有出错，继续执行后续代码
        parseRequestLine(); // 解析请求行，把 请求方法 uri 协议版本 搞出来
        ParseRequestHeader();// 将请求报头中的KV全部放到哈希中，方便查找
        RecvRequestBody();// 分析请求行中是POST还是GET，是POST则需要接收正文，是GET则不需要，但统一走接收正文的接口
        if(stop == false)
        {
          LOG(INFO, "recv client's request success");
        }
        else
        { // 接受请求正文出错
          LOG(WARNING, "recv requestBody failed");
          end = true;
        }
      }
      else
      { // 接收请求报头 + 空行出错
        end = true;
      }
    }
    else
    { // 接收请求行出错
      end = true;
    }

    //cout << _httpRequest._requestLine << endl;
    // cout << _httpRequest._requestHeader << endl;
    //cout << _httpRequest._requestBody << endl;
    
    return end;
  }

private: // 响应内部接口
  // 分析是GET还是POST，是POST参数就全在正文body里，是GET则在uri里

  size_t ProcessCgi()
  {
    /* cgi整体流程：创建子进程，让cgi进程替换掉子进程，父进程将
    request中的参数传给cgi进程，cgi进程处理完后将结果返回给父进程 */

    // 走这里request一定是POST或者带参的GET方法，那么此时request请求中的path就是所需要的可执行程序的路径，此时就要让这个可执行程序替换掉这里的子进程
    auto &path = _httpRequest._path; // 可执行程序的路径

    // 想要让父子进程通信，直接用管道来实现
    int input[2]; // 站在父进程角度的输入和输出，父进程用input[0]读，用output[1]写
    int output[2];

    if(pipe(input) < 0) // 创建input管道
    { // 创建input管道失败
      LOG(ERROR, "create input pipe failed, strerror: " + std::string(strerror(errno)));
      return 500;
    }

    if(pipe(output) < 0) // 创建output管道
    { // 创建output管道失败
      LOG(ERROR, "create output pipe failed, strerror: " + std::string(strerror(errno)));
      return 500;
    }

     // 走到这里管道就创建好了，此时创建一个子进程就可以进行父子进程通信了

    // 创建子进程
    pid_t pid = fork();
    if(pid == 0)
    { // 子进程
       // 关掉不需要的文件描述符
       close(input[0]);
       close(output[1]);

       // ============================================== 带参GET用环境变量传递参数
      if(_httpRequest._method == "GET")
      { // 方法是带参GET
        // 将queryString转到环境变量中
        std::string queryString_env = "QUERY_STRING=" + _httpRequest._queryString;
        // LOG(INFO, "QUERY_STRING is-->" + queryString_env);
        if(putenv((char*)queryString_env.c_str()) != 0)
        {
          LOG(ERROR, "putenv queryString failed");
          return 500;
        }
      }
      else if(_httpRequest._method == "POST")
      { // POST方法，要指明传入了多少字节的参数，数字不会很大，也用环境变量来给
        std::string contentLength_env = "CONTENT_LENGTH=" + std::to_string(_httpRequest._contentLenth);
        if(putenv((char*)contentLength_env.c_str()) != 0)
        {
          LOG(ERROR, "POST method, putenv CONTENT_LENGTH failed");
          return 500;
        }
      }

      // 替换进程要根据POST或带参GET来判断是从管道中读取还是从环境变量导入，所以要让其知道method是啥，而method不是很长，所以就导入环境变量即可
      std::string method_env = "METHOD=" + _httpRequest._method;
      if(putenv((char*)method_env.c_str()) != 0)
      {
        LOG(ERROR, "putenv method failed");
        return 500;
      }

       // 进程替换之后会导致用户层的数据丢失，此时替换进程无法找到先前打开的匿名管道的fd，所以要重定向一下
      dup2(output[0], 0); // 替换之后的进程，文件描述符0就表示原先的output[0]，也就是子进程专门读取的管道的文件描述符
      dup2(input[1], 1); // 替换之后的进程，文件描述符1就表示原先的input[1]，也就是子进程专门读取的管道的文件描述符

      /* 进程替换的时候直接用path就可以直接找到对应文件，在命令行上调用的方法也是path，比如说
      path为wwwroot/testCgi，那么命令行上执行这个文件的方式就是./wwwroot/testCgi，而第execl
      的二个参数传参的时候不需要在path前面加上./，这个函数实现的就是如此，加不加./都是能跑的 */
      execl(path.c_str(), path.c_str(), nullptr); // 调用execl替换掉子进程

      LOG(ERROR, "execl failed");
      exit(1); // 如果替换异常就会执行这句，此时子进程的退出码就是1，根据这个来让父进程判断子进程是否成功替换
               // 如果父进程发现子进程退出码是1，那就是替换失败了
    }
    else if(pid > 0)
    {// 父进程
        // 关掉不用的文件描述符
      close(input[1]);
      close(output[0]);
      
      // ============================================== POST用管道传递参数
      if(_httpRequest._method == "POST")
      { // 方法是POST，将body中的内容全部写到output[1]中
        size_t total = 0, size_singleTime = 0;
        size_t size = _httpRequest._requestBody.size();
        while(total < size && (size_singleTime = write(output[1], _httpRequest._requestBody.c_str(), size) > 0))
        {
          total += size_singleTime;
        }

        if(total != size)
        {
          LOG(ERROR, "write size[" + std::to_string(total) + "] != body size[" + std::to_string(size) + ']');
          return 500;
        }
      }

      // 发送完数据之后就接收CGI进程返回的结果
      char ch;
      while(read(input[0], &ch, 1) > 0)
      {
        _httpResponse._responseBody.push_back(ch);
      }
      cout << "-->" << "server get result:\n" << _httpResponse._responseBody << endl;

      int status = 0; // 父进程还是要等待替换掉的进程
      pid_t _pid = waitpid(-1, &status, 0);
      if(pid == _pid)
      {
        if(WIFEXITED(status))
        { // 正常退出，即调用exit、_exit、return退出的
          if(WEXITSTATUS(status) != 0)
          { // 退出码不为0，上面替换失败了，或者是替换后的进程退出码不为零，返回
            cout << "exit code ::" << WEXITSTATUS(status) << endl;
            return 500;
          }
          else
          {
            cout << "exit code ::" << WEXITSTATUS(status) << endl;
            // 正常退出，退出码为0，最终返回的就是OK
          }
        }
        else
        { // 子进程没有正常退出，可能是信号终止了啥的
          return 400;
        }
      }
      else
      { // 就一个子进程，如果等待到的进程id和前面子进程的id不同，那就出错了
        return 500;
      }

      // 记得最后要关闭文件描述符
      close(input[0]);
      close(output[1]);
      // 子进程替换之后会执行完毕，此时替换进程整个空间都会被回收，所以替换进程中可以不需要手动关input[1]和output[0]
    }
    else
    { // 创建子进程失败
      return 500;
    }
    

    return OK;
  }

  size_t ProcessNonCgi()
  { // 非cgi处理，就是直接返回一个页面
      // 需要先构建一下Response，然后再由SendResponse发送出去
      // 响应格式如下：
      //   状态行：协议版本 状态码 状态描述\r\n
      // 响应报头：一堆K: V\r\n
      //     空行：\r\n
      // 响应正文：某个文件中的内容\r\n


    // 想要将文件中的内容发送出去，那就要先打开request中的path对应文件
    _httpResponse._fd = open(_httpRequest._path.c_str(), O_RDONLY); // 以读方式打开文件
    if(_httpResponse._fd >= 0) // 打开成功才能发出去
    {
      return OK; // 文件能找到
    }
    
    return NOT_FOUND;
  }

  // 请求正确
  void HandlerOK()
  { // 当状态码是OK的时候构建一下响应正文
    //________________________状态行
    _httpResponse._statusLine += _httpRequest._version;
    _httpResponse._statusLine += ' ';
    _httpResponse._statusLine += std::to_string(_httpResponse._statusCode);
    _httpResponse._statusLine += ' ';
    _httpResponse._statusLine += StatusCodeToDesc(_httpResponse._statusCode);
    _httpResponse._statusLine += END_OF_LINE;

    //________________________响应报头
    std::string tmp = "Content-Length: ";
    if(_httpRequest._cgi == true)
    { // cgi模式，响应正文在前面ProcessCgi中已经直接读取到了_httpResponse._responseBody中，直接用_responseBody.size()即可
      tmp += std::to_string(_httpResponse._responseBody.size());
    }
    else
    { // 非cgi模式，响应正文就是前面BuildResponse中打开的静态页面文件，大小也就是那个stat.st_size
      tmp += std::to_string(_httpRequest._size);
    }
    tmp += END_OF_LINE;
    _httpResponse._responseHeader.push_back(tmp);

    tmp = "Content-Type: ";
    tmp += SuffixToDesc(_httpRequest._suffix);
    tmp += END_OF_LINE;
    _httpResponse._responseHeader.push_back(tmp);

    //________________________空行已经在构造函数中初始化
    //________________________响应正文。
      // 对于cig而言，响应正文在前面ProcessCgi中已经直接读取到了_httpResponse._responseBody中；
      // 对于非cgi而言，这里不用再打开文件再拷贝到用户层，后面在给Client发送的时候直接用sendfile函数，不经过用户层直接发出。
    
    // 构造完毕响应报文，接着用SendResponse发送出去
  }

  void HandlerERROR(const std::string& page)
  {
    _httpRequest._cgi = false; // 如果进入这里就要将cgi设置为false，不然影响后续发送时判断cgi
    std::string pagePath = WEB_ROOT;
    pagePath += '/';
    pagePath += page;
    _httpResponse._fd = open(pagePath.c_str(), O_RDONLY);
    if(_httpResponse._fd >= 0)
    {
      //________________________状态行
      _httpResponse._statusLine += _httpRequest._version;
      _httpResponse._statusLine += ' ';
      _httpResponse._statusLine += std::to_string(_httpResponse._statusCode);
      _httpResponse._statusLine += ' ';
      _httpResponse._statusLine += StatusCodeToDesc(_httpResponse._statusCode);
      _httpResponse._statusLine += END_OF_LINE;

      //________________________响应报头
      std::string tmp = "Content-Length: ";
      // 获取错误页面大小
      struct stat st;
      stat(pagePath.c_str(), &st);
      tmp += std::to_string(st.st_size);
      tmp += END_OF_LINE;
      _httpResponse._responseHeader.push_back(tmp);

      tmp = "Content-Type: ";
      tmp += SuffixToDesc(_httpRequest._suffix);
      tmp += END_OF_LINE;
      _httpResponse._responseHeader.push_back(tmp);

      // 正文部分和非cgi的处理方式一样，调用sendfile即可，不过要将读取的fd设置一下
      _httpRequest._size = st.st_size;
    }
  }

  // 构建响应
  void _BulidResponse()
  {
    switch(_httpResponse._statusCode)
    {
    case OK:
      HandlerOK();
      break;
    case NOT_FOUND:
      HandlerERROR(NOT_FOUND_PAGE);
      break;
    case BAD_REQUEST:
      HandlerERROR(BAD_REQUEST_PAGE); // 这里的NOT_FOUND_PAGE应该是再写一个BAD_REQUEST对应页面的，但是懒得搞了，就这样
      break;
    case SERVER_ERROR:
      HandlerERROR(SERVER_ERROR_PAGE); // 这里的NOT_FOUND_PAGE应该是再写一个NOT_FOUND_PAGE对应页面的，但是懒得搞了，就这样
      break;
    default:
      HandlerERROR(NOT_FOUND_PAGE);
      break;
    }
  }
  
public:
  void BuildResponse() // 请读取完毕，分析请求，构建响应
  { // 构建响应的状态码决定了报文的处理方式
    /* 所有的请求方法暂时只考虑GET和POST，其余的禁掉。
          若请求方法是GET，那么可能是在获取资源，也可能是在提交数据，
          若请求方法是POST，那么就是在提交数据， 
          如果请求的资源是一个可执行程序，则和带参GET、POST方法一样，都用Cgi解决*/
    auto &method = _httpRequest._method; // 请求方法
    struct stat fileStat; // stat函数的第二个参数，用来获取文件信息
    size_t findSuffix = 0;
    if(method != "GET" && method != "POST")
    { // 既不是GET也不是POST，直接禁掉
      LOG(WARNING, "client method is :" + method);
      _httpResponse._statusCode = BAD_REQUEST; // 先设置成NOT_FOUND，后面再改其他的-------------------------------
      goto END;
    }
    else if(method == "GET") // GET方法，分为uri带参的和不带参的
    {
      size_t pos = _httpRequest._uri.find('?');
      if(pos != std::string::npos)
      { // 找到了一个带参的uri，即path:arguments，切分一下
        if(Util::CutString(_httpRequest._uri, _httpRequest._path, _httpRequest._queryString, "?"))
        { // 切分成功
          //cout << "-------------" << _httpRequest._path << ":" << _httpRequest._queryString << endl;
          _httpRequest._cgi = true;
        }
        else
        {
          LOG(ERROR, "uri have arguments, but split path:arguments failed");
        }
      }
      else
      { // 不带参的uri
        _httpRequest._path = _httpRequest._uri; // path就是uri，没有_queryString
      }
    }
    else
    { // POST方法
      _httpRequest._cgi = true;
      _httpRequest._path = _httpRequest._uri; // POST方法中uri没有参数，直接获取就是path
    }

    // 修正path
    _httpRequest._path = WEB_ROOT + _httpRequest._path;// Client发来的都是/a/b……，要有一个专属的Web根目录，所以需要在前面加上wwwroot
    
    if(_httpRequest._path[_httpRequest._path.size() - 1] == '/')
    { // 两种情况：
      // 1. 请求的就是Web根目录，即/wwwroot/
      // 2. 请求的是某一个子目录，即/wwwroot/a/，类似这样的方式
      // 不管是哪种目录，都在后面加上index.html，设置为主页，不管是根目录还是普通目录都返回的是主页
      
      _httpRequest._path += HOME_PAGE;
    }

    // 到这里的文件,要么有后缀，要么没有后缀

    // 看看这个文件在不在
    if(stat(_httpRequest._path.c_str(), &fileStat) == -1)
    { // 没有找到这个文件
      LOG(ERROR, "path " + _httpRequest._path + " NOT_FOUND, err:" + std::string(strerror(errno)));
      _httpResponse._statusCode = NOT_FOUND; // 找不到文件
      goto END;
    }
    else
    { // 文件存在，保存一下文件相关属性

      // 文件存在，但是一个可执行文件
      if((fileStat.st_mode & S_IXUSR) || (fileStat.st_mode & S_IXGRP) || (fileStat.st_mode & S_IXOTH))
      {
        _httpRequest._cgi = true;
      }
      else if(S_ISDIR(fileStat.st_mode)) // 文件存在但是是一个目录 如：/reference/string ==> string是目录的话就应该访问其中的默认页面
      {
        _httpRequest._path += '/';
        _httpRequest._path += HOME_PAGE;
        stat(_httpRequest._path.c_str(), &fileStat);
      }
      
      _httpRequest._size = fileStat.st_size; // 对应文件的大小，后面返回静态网页的时候会用到
    }

    // 这里是能找到文件，此时再分两种情况，即有没有后缀
    findSuffix = _httpRequest._path.rfind('.');
    if(findSuffix == std::string::npos)
    { // 没有后缀，统一按照html处理
      _httpRequest._suffix = "html";
    }
    else
    { // 有后缀，设置好后缀
      _httpRequest._suffix = _httpRequest._path.substr(findSuffix + 1);
    }


    // 最后看如何处理这个文件，是cgi还是非cgi
    if(_httpRequest._cgi == true) // cgi
    { 
      _httpResponse._statusCode = ProcessCgi();
    }
    else // 非cgi，就返回一个普通网页
    {
      _httpResponse._statusCode = ProcessNonCgi();
    }

END:
    // 已经设置完毕错误码，下面就是根据错误码来决定响应报头和正文
    _BulidResponse();

    // 至此报文构建完毕，后续进行发送

    // cout << "----------------" << "cgi: " << _httpRequest._cgi << endl;
    // cout << "----------------" << "status line: " << _httpResponse._statusLine;
    // cout << "----------------" << "build response over" << endl;
    return;
  }

  void SendResponse() // 返回响应
  {
    send(_sock, _httpResponse._statusLine.c_str(), _httpResponse._statusLine.size(), 0); // 发送状态行

    for(auto &str : _httpResponse._responseHeader) // 发送响应报头
    {
      send(_sock, str.c_str(), str.size(), 0);
    }
    
    send(_sock, _httpResponse._blank.c_str(), _httpResponse._blank.size(), 0); // 发送空行
    
    if(_httpRequest._cgi == true)
    { // 是cgi，就直接发送_httpResponse._responseBody中的内容
      size_t total = 0, size = 0;
      while((total < _httpResponse._responseBody.size()) && ((size = send(_sock, _httpResponse._responseBody.c_str(), _httpResponse._responseBody.size(), 0)) > 0))
      {
        total += size;
      }
    }
    else
    {
      // 这个函数可以不用将数据拿到用户层，直接让内核层进行操作，拿出文件 _httpResponse._fd 中 _httpResponse._size 的内容放到_sock对应的发送缓冲区中
      sendfile(_sock, _httpResponse._fd, nullptr, _httpRequest._size); // 发送响应正文
    }
    // 这些send函数的功能不过是拷贝而已，仅仅是将用户层要发送的数据交给内核层，后面什么时候发送、一次发多少都是应用层之下的协议决定的
  }
};

class ThreadCallBack
{
  public:
    void operator()(int sock)
    {
      Handler(sock);
    }

    static void* Handler(int sock)
    {
      LOG(INFO, "thread[" + std::to_string(pthread_self()) + "] handling sock[" + std::to_string(sock) + "] begin");

//#define DEBUG 1
#ifdef DEBUG      
      // 读取一下浏览器发来的HTTP协议
      char buff[1024];
      size_t size = read(sock, buff, sizeof(buff));
      if(size > 0)
      { // 读取到数据
        buff[size] = 0;
        cout << "--------------------------------------" << endl;
        cout << buff << endl;
        cout << "--------------------------------------" << endl;
      }
      else if(size == 0)
      { // 对端关闭
        cout << "client closed" << endl;
      }
      else 
      {
        cout << "read err" << endl;
      }
#else
      std::shared_ptr<EndPoint> ep(new EndPoint(sock));
      if(ep->RecvRequest())
      { // read时对端关闭连接，出错，后续不用再执行了
        return nullptr;
      }
      ep->BuildResponse();
      ep->SendResponse();
#endif
      LOG(INFO, "thread[" + std::to_string(pthread_self()) + "] handling sock[" + std::to_string(sock) + "] over");
      return nullptr;
    }

};
