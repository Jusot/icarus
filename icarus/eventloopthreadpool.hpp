#ifndef ICARUS_EVENTLOOPTHREADPOOL_HPP
#define ICARUS_EVENTLOOPTHREADPOOL_HPP

#include <vector>
#include <memory>
#include <functional>

#include "noncopyable.hpp"

namespace icarus
{
class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
  public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop *base_loop);
    EventLoopThreadPool(EventLoop *base_loop, int num_threads);
    ~EventLoopThreadPool();

    void set_thread_num(std::size_t num_threads);
    void start(const ThreadInitCallback &cb = ThreadInitCallback());
    EventLoop *get_next_loop();

  private:
    EventLoop *base_loop_;
    bool started_;
    int num_threads_;
    std::size_t next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop *> loops_;
};

} // namespace icarus

#endif // ICARUS_EVENTLOOPTHREADPOOL_HPP