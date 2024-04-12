#pragma once

#include"LOG.hpp"
#include"Task.hpp"

#include<queue>

#include<pthread.h>

const static size_t NUM = 6;

class ThreadPool
{
private:
    pthread_mutex_t _mtx; // 锁
    pthread_cond_t _cond; // 条件变量
    size_t _num; // 线程个数
    std::queue<Task*> _taskQueue;

    static ThreadPool* _tp; // 单例模式
private:
    ThreadPool(size_t num = NUM) // 主要是确定线程个数
      : _num(num)
    {
        pthread_mutex_init(&_mtx, nullptr);//初始化
        pthread_cond_init(&_cond, nullptr);
    }
    
    ~ThreadPool()
    {
        pthread_mutex_destroy(&_mtx);
        pthread_cond_destroy(&_cond);
    }

    // 成员_mtx不是静态成员，不能直接在Routine中直接调用_mtx，如果想用_mtx还得要在前面加上ptp->，所以上锁和解锁的接口就要手动提供一下
    void Lock()
    {
        pthread_mutex_lock(&_mtx);
    }

    void Unlock()
    {
        pthread_mutex_unlock(&_mtx);
    }

    // 同样，成员_cond也不是静态成员，也要提供一下接口
    void WaitForCond() //线程在_cond上等待
    {
        pthread_cond_wait(&_cond, &_mtx);
    }

    void WakeThreadInCond() // 每次唤醒一个线程
    {
        pthread_cond_signal(&_cond);
    }

    // 从线程取任务
    void PopTask(Task* &task) // 输出型参数，外面获取队头任务
    {
        task = _taskQueue.front();
        _taskQueue.pop();
    }

    // 线程执行方法
    static void* ThreadRountine(void* args)
    { // 新线程不做记录，直接死循环从任务队列中拿数据就行
        pthread_detach(pthread_self()); // 线程分离，不需要管理，主程序执行完毕，线程仍会执行
        ThreadPool* ptp = reinterpret_cast<ThreadPool*>(args); // 获取当前线程池的指针，从而调用某些接口
        
        // 等条件变量，有条件了就从任务队列中取任务
        while(true)
        {
            ptp->Lock();
            // 用while防止误调用pthread_cond_signal导致线程唤醒，醒来后再判断一下队列中有没有任务，如果队列中没有任务就进入循环继续等，有任务就跳出循环
            while(ptp->_taskQueue.empty()) 
            {
                ptp->WaitForCond();
            }
            // 到这里线程就要取任务来执行
            Task* t;
            ptp->PopTask(t);
            ptp->Unlock();

            t->PorcessOn(); // 执行任务

            delete t;
            // 执行完毕后循环上去继续等任务到来
        }
    }

    void ThreadPoolInit()
    {
        LOG(INFO, "ThreadPool start to init");
        for(size_t i = 0; i < _num; ++i)
        {
            pthread_t tid;
            if(pthread_create(&tid, nullptr, ThreadRountine, this) != 0)
            {
                LOG(ERROR, "ThreadPool init failed(thread create failed)");
                exit(5);
            }
        }
        LOG(INFO, "ThreadPool init success");
    }
public:
    // 单例模式
    static ThreadPool* GetInstance()
    {
        static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER; // 静态的锁可以直接这样初始化
        if(_tp == nullptr)
        {
            pthread_mutex_lock(&mtx);
            if(_tp == nullptr)
            {
                _tp = new ThreadPool();
                _tp->ThreadPoolInit();
            }
            pthread_mutex_unlock(&mtx);
        }

        return _tp;
    }

    // 主线程放任务
    void PushTask(Task* task)
    {
        pthread_mutex_lock(&_mtx);
        _taskQueue.push(task);
        pthread_mutex_unlock(&_mtx);
        WakeThreadInCond();
        LOG(INFO, "push task success");
    }
};

ThreadPool* ThreadPool::_tp = nullptr;
