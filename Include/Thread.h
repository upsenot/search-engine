#ifndef __THREAD_H__
#define __THREAD_H__

#include <pthread.h>
#include <functional>
#include <climits>

using std::function;
using std::bind;

using ThreadCallback = function<void()>;
extern __thread size_t threadIdx;
class Thread
{
public:
    Thread(ThreadCallback &&cb,size_t threadIdx = INT_MAX);
    ~Thread();
    void start();
    void join();

private:
    //线程入口函数
    static void *threadFunc(void *arg);

private:
    pthread_t _thid;
    size_t _threadIdx;
    bool _isRunning;
    //要去实现的任务
    ThreadCallback _cb;

};

#endif
