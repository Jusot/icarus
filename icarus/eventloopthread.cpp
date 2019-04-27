#include <cassert>

#include "eventloop.hpp"
#include "eventloopthread.hpp"

using namespace icarus;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb)
  : loop_(nullptr), 
    exiting_(false), 
    callback_(cb)
{
    // ...
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = false;
    if (loop_)
    {
        loop_->quit();
        thread_.join();
    }
}

EventLoop *EventLoopThread::start_loop()
{
    assert(!thread_.joinable());
    thread_ = std::thread(std::bind(&EventLoopThread::thread_func, this));
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (!loop_)
        {
            cond_.wait(lock, [&] { return loop_; });
        }
    }
    return loop_;
}

void EventLoopThread::thread_func()
{
    EventLoop loop;

    if (callback_)
    {
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_all();
    }

    loop.loop();
    std::lock_guard<std::mutex> lock(mutex_);
    loop_ = nullptr;
}