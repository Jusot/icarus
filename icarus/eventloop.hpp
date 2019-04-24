#ifndef ICARUS_EVENTLOOP_HPP
#define ICARUS_EVENTLOOP_HPP

#include <mutex>
#include <vector>
#include <thread>
#include <memory>
#include <functional>

#include "noncopyable.hpp"

namespace icarus
{
class Channel;
class Poller;

class EventLoop : noncopyable
{
  public:
    using Functor = std::function<void()>;

    // default contructor
    EventLoop();

    // default deconstructor
    ~EventLoop();

    // main loop
    void loop();

    // quits loop
    void quit();

    void run_in_loop(Functor cb);

    void queue_in_loop(Functor cb);

    std::size_t queue_size() const;

    void wakeup();

    void update_channel(Channel *channel);

    // assert current thread is in loop thread or not
    void assert_in_loop_thread();

    // checks current thread is in loop thread or not
    bool is_in_loop_thread() const;

    // returns EventLoop object's address of current thread
    static EventLoop *get_event_loop_of_current_thread();

  private:
    // abort when not in loop thread
    void abort_not_in_loop_thread();
    void handle_read();
    void do_pending_functors();

    using ChannelList = std::vector<Channel *>;

    bool looping_;
    bool quit_;
    bool calling_pending_functors_;
    const std::thread::id thread_id_;
    std::unique_ptr<Poller> poller_;
    int wakeup_fd_;
    std::unique_ptr<Channel> wakeup_channel_;
    ChannelList active_channels_;
    std::mutex mutex_;
    std::vector<Functor> pending_functors_;
};
} // namespace icarus

#endif // ICARUS_EVENTLOOP_HPP