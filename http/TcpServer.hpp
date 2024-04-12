#pragma once

#include "Common.h"

const static int WAIT_LENGTH = 5; // 全连接队列长度为  WAIT_LENGTH + 1

class TcpServer
{
private:
  int _port;       // 端口
  int _listenSock; // 监听套接字
//使用单例模式可以确保只有一个服务器实例存在，避免了多个服务器实例之间的端口冲突或资源竞争问题
  // 懒汉模式是在需要时才创建实例(本代码用的懒汉模式)
  //恶汉模式：是在类加载时就创建实例
private:
  static TcpServer *_tsvr; // 单例模式
//单例类的构造函数必须是私有的
  TcpServer(int port)
      : _port(port), _listenSock(-1)
  {}
  //{} 初始化列表是一种在C++中用来初始化对象的便捷方式，可以适用于各种类型的对象
  //例如：student stu{3,"李","五班"};

//禁止拷贝构造函数和赋值运算符
  TcpServer(const TcpServer &cp) = delete;
  TcpServer &operator=(const TcpServer &cp) = delete;

public:
//GetInstance 函数来获取单例实例，确保了在程序运行期间只有一个 TcpServer 实例存在
  static TcpServer *GetInstance(int port)
  {  //判断 _tsvr 是否为空（即 nullptr）是为了确定是否已经有一个实例被创建。如果 _tsvr 不为空，说明已经有一个实例存在，就不需要再创建新的实例了，而是直接返回已经存在的实例。
    // 双锁机制，双if ，效率高且安全，确保在多线程环境下只创建一个实例
    if (_tsvr == nullptr)
    {
      static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
      // linux里面的static的锁相当于全局的锁，PTHREAD_MUTEX_INITIALIZER 是一个宏，用于初始化静态的互斥锁。
      // 获取静态的互斥锁 mtx。这是为了确保在多线程环境下只有一个线程能够执行创建实例的代码，避免多个线程同时创建多个实例。
      pthread_mutex_lock(&mtx);
      if (_tsvr == nullptr)
      {
        _tsvr = new TcpServer(port);//这个就是创建后的实例，意味着获取了一个具体的服务器对象
      }
      pthread_mutex_unlock(&mtx);
    }

    return _tsvr;
  }

  void InitServer()
  {
    Socket(); // 创建监听套接字
    Bind();   // 绑定IP与Port
    Listen(); // 设置监听状态
  }

  // 创建套接字
  void Socket()
  {
    //      网络通信ipv4    流式传输    给0默认协议，此时为TCP
    _listenSock = socket(AF_INET, SOCK_STREAM, 0);//监听的文件描述符
    if (_listenSock < 0)
    { // 创建监听套接字失败
      LOG(FATAL, "create listen sock fail");
      exit(1);
    }
    int opt = 1;
    // 设置地址复用，防止服务器崩掉后进入TIME_WAIT，短时间连不上当前端口
    //这样可以允许多个进程或应用程序同时监听相同的端口
    setsockopt(_listenSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    LOG(INFO, "create listen sock success");
  }

  // bind绑定
  void Bind()
  {
    sockaddr_in local;
    memset(&local, 0, sizeof(local));   // 给sin_zero置0
    local.sin_family = AF_INET;         // 协议家族和创建socket时相同
    local.sin_port = htons(_port);      // 主机转网络字节序
    local.sin_addr.s_addr = INADDR_ANY; // 将服务端IP直接给为0.0.0.0，能接受到任意网卡传来的信息
    if (bind(_listenSock, (sockaddr *)&local, sizeof(local)) < 0)
    { // 绑定失败
      LOG(INFO, "bind fail");
      exit(2);
    }
    LOG(INFO, "bind success");
  }

  // listen设置监听状态
  void Listen()
  {//WAIT_LENGTH开头定义了 监听的最大值
    if (listen(_listenSock, WAIT_LENGTH) < 0)
    { // 监听失败
      LOG(INFO, "listen fail");
      exit(3);
    }
    LOG(INFO, "listen success");
  }

  int ListenSocket()
  {
    return _listenSock;//返回监听的文件描述符
  }

  ~TcpServer()
  {
    if(_listenSock > 0)
    {
      close(_listenSock);
    }
  }
};

TcpServer *TcpServer::_tsvr = nullptr; // 单例模式对象
//TcpServer:: 表示这个静态成员指针 _tsvr 是属于 TcpServer 类的。
//TcpServer * 表示 _tsvr 是一个指向 TcpServer 类对象的指针。
