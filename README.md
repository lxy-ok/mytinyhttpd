# 我
■	mytinyhttpd  C++    

项目简介：实现WEB端用户注册、登录功能；实现分级日志系统，记录服务器运行状态；支持解析客户端GET和POST请求的小型服务器。

关键特性：

•	利用epoll实现IO多路复用，单例模式（懒汉模式）实现线程池；

•	利用C++11中的condition_variable实现线程的等待和通知机制，从而在多线程环境中实现同步操作；

•	利用map管理http连接取代固定数组，用智能指针管理连接生命期，避免串话和内存泄漏。

日志：

![image](https://github.com/lokokokokokoko/lok/assets/154768611/ba53d5c8-b32a-4e71-88e6-677de13a1bd9)

......



