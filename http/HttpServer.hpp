#pragma once
#include "TcpServer.hpp"
#include "Protocol.hpp"
#include "ThreadPool.hpp"

const static int PORT = 8081; // 默认端口

    /*线程池版本*/
class HttpServer
{
private:
  int _port;  // 通过上层设置端口
  bool _stop; // 停止服务，默认值false，表示不停止

public:
  HttpServer(int port = PORT)
      : _port(port), _stop(false)
  {}
//循环，用于接受客户端的连接并将连接放入任务队列中由线程池处理
  void Loop()
  {/*如果进程向一个已关闭的管道写入数据，系统会向该进程发送 SIGPIPE 信号,
  通过调用 signal(SIGPIPE, SIG_IGN)，可以将对 SIGPIPE 信号的默认行为设置为忽略，即当进程向已关闭的管道写入数据时
  系统不会发送 SIGPIPE 信号，而是简单地返回一个错误码。*/
    signal(SIGPIPE,SIG_IGN); 
//服务端写客户端关闭，此时会导致服务端进程接收到SIGPIPE新型号，直接被os杀掉，忽略这个信号，防止因这种情况导致服务端关闭
//客户端关闭后不会导致服务器崩溃

    //端口传了过去
    //GetInstance() 函数返回的是 TcpServer 类的一个指针，而指针可以用来访问其指向的对象的成员函数和成员变量。
    TcpServer::GetInstance(_port)->InitServer(); // 前三步，socket、bind、listen
    LOG(INFO, "server start to access links");

    // 接收连接并执行任务,默认值false，表示不停止
    while (!_stop)
    {
      sockaddr_in peer;               // 对端信息
      memset(&peer, 0, sizeof(peer)); // 初始化0
      socklen_t len = sizeof(peer);
    //这个函数是一个阻塞函数，当没有新的客户端连接请求的时候，该函数阻塞；当检测到有新的客户端连接请求时，阻塞解除，新连接就建立了
    //得到的返回值也是一个文件描述符，基于这个文件描述符就可以和客户端通信了。
      // 接受连接
      int sock = accept(TcpServer::GetInstance(_port)->ListenSocket(), (sockaddr *)&peer, &len);
      if (sock < 0)
      { // 接收连接失败，继续接收下一个
        LOG(ERROR, "get new link failed");
        continue;
      }
      // 获取客户端IP和port， inet_ntoa大端整形 -> 点分十进制IP
      std::string clientIP = inet_ntoa(peer.sin_addr);
      int clientPort = ntohs(peer.sin_port);//ntohs将一个整形从网络字节序 -> 主机字节序

      // 打印一下客户端信息
      LOG(INFO, std::move("get new link, sock[" + std::to_string(sock) + "], client message::" + clientIP + ":" + std::to_string(clientPort)));

      //接收到链接后将任务放置到任务队列中，让线程池中的某个线程去执行任务
      Task* t = new Task(sock);
      ThreadPool::GetInstance()->PushTask(t);
    }
  }
};



    /*新来链接就创建一个线程的版本*/
// class HttpServer
// {
// private:
//   int _port;  // 通过上层设置端口
//   bool _stop; // 停止服务，默认值false，表示不停止

// public:
//   HttpServer(int port = PORT)
//       : _port(port), _stop(false)
//   {}

//   void Loop()
//   {
//     signal(SIGPIPE, SIG_IGN); // 服务端写客户端关闭，此时会导致服务端进程接收到SIGPIPE新型号，直接被os杀掉，忽略这个信号，防止因这种情况导致服务端关闭

//     TcpServer *tsvr = TcpServer::GetInstance(_port);
//     tsvr->InitServer(); // 前三步，socket、bind、listen
//     LOG(INFO, "server start to access links");

//     // 接收连接并执行任务
//     while (!_stop)
//     {
//       sockaddr_in peer;               // 对端信息
//       memset(&peer, 0, sizeof(peer)); // 初始化0
//       socklen_t len = sizeof(peer);

//       // 接受连接
//       int sock = accept(tsvr->ListenSocket(), (sockaddr *)&peer, &len);
//       if (sock < 0)
//       { // 接收连接失败，继续接收下一个
//         LOG(ERROR, "get new link failed");
//         continue;
//       }
//       // 获取客户端IP和port
//       std::string clientIP = inet_ntoa(peer.sin_addr);
//       int clientPort = ntohs(peer.sin_port);

//       // 打印一下客户端信息
//       LOG(INFO, std::move("get new link, client message::" + clientIP + ":" + std::to_string(clientPort)));

//       // 接收到新连接后，新起一个线程，让新线程去处理新连接的任务
//       int *p = new int(sock);
//       pthread_t tid;
//       pthread_create(&tid, nullptr, ThreadCallBack::Handler, (void *)p);

//       pthread_detach(tid); // 线程分离，os自动回收新线程，不需要让主线程join等待
//     }
//   }
// };
