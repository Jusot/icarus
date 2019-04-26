#ifndef ICARUS_EVENTLOOPTHREAD_HPP
#define ICARUS_EVENTLOOPTHREAD_HPP

#include <mutex>
#include <string>
#include <thread>
#include <functional>
#include <condition_variable>

#include "noncopyable.hpp"

namespace icarus
{
class EventLoop;

class EventLoopThread : noncopyable
{
  public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback());
    ~EventLoopThread();
    
    EventLoop *start_loop();

  private:
    void thread_func();

    EventLoop *loop_;
    bool exiting_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};

} // namespace icarus

#endif // ICARUS_EVENTLOOPTHREAD_HPP