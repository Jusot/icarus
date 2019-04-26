#include <cassert>

#include "eventloop.hpp"
#include "eventloopthread.hpp"
#include "eventloopthreadpool.hpp"

using namespace icarus;

EventLoopThreadPool::EventLoopThreadPool(EventLoop *base_loop)
  : base_loop_(base_loop),
    started_(false),
    num_threads_(0),
    next_(0)
{
    // ...
}

EventLoopThreadPool::EventLoopThreadPool(EventLoop *base_loop, int num_threads)
  : base_loop_(base_loop),
    started_(false),
    num_threads_(num_threads),
    next_(0)
{
    // ...
}

EventLoopThreadPool::~EventLoopThreadPool()
{
    // ...
}

void EventLoopThreadPool::set_thread_num(int num_threads)
{
    num_threads_ = num_threads;
}

void EventLoopThreadPool::start(const ThreadInitCallback &cb)
{
    assert(!started_);
    base_loop_->assert_in_loop_thread();

    started_ = true;

    for (int i = 0; i < num_threads_; ++i)
    {
        threads_.push_back(std::make_unique<EventLoopThread>(cb));
        loops_.push_back(threads_.back()->start_loop());
    }

    if (num_threads_ == 0 && cb)
    {
        cb(base_loop_);
    }
}

EventLoop* EventLoopThreadPool::get_next_loop()
{
    base_loop_->assert_in_loop_thread();
    assert(started_);
    
    auto loop = base_loop_;
    if (!loops_.empty())
    {
        loop = loops_[next_];
        if (++next_ >= loops_.size())
        {
            next_ = 0;
        }
    }
    return loop;
}