#pragma once

#include"Protocol.hpp"

class Task
{
private:
    int _sock; // 通信的fd
    ThreadCallBack _cb;

public:
    Task(int sock = -1) // 调用ThreadCallback的时候要用到通信的sock
        : _sock(sock)
    {
        cout << "=================> Task start, sock[" << _sock << "]" << endl;
    }

    ~Task()
    {
        cout << "=================> ~Task(), sock[" << _sock << "]" << endl;
        if(_sock >= 0) close(_sock);
    }

    void PorcessOn()
    {
        _cb(_sock); // 调用ThreadCallback的仿函数，从而直接调用处理Http请求的逻辑
    }
};