#ifndef ICARUS_EVENTLOOP_HPP
#define ICARUS_EVENTLOOP_HPP

#include <vector>
#include <thread>
#include <memory>

#include "noncopyable.hpp"

namespace icarus
{
class Channel;
class Poller;

class EventLoop : noncopyable
{
  public:
    // default contructor
    EventLoop();

    // default deconstructor
    ~EventLoop();

    // main loop
    void loop();

    // quits loop
    void quit();

    // assert current thread is in loop thread or not
    void assert_in_loop_thread();

    // checks current thread is in loop thread or not
    bool is_in_loop_thread() const;

    // returns EventLoop object's address of current thread
    static EventLoop *get_event_loop_of_current_thread();

  private:
    // abort when not in loop thread
    void abort_not_in_loop_thread();

    using ChannelList = std::vector<Channel *>;

    bool looping_;
    bool quit_;
    const std::thread::id thread_id_;
    std::unique_ptr<Poller> poller_;
    ChannelList active_channels_;
};
} // namespace icarus

#endif // ICARUS_EVENTLOOP_HPP