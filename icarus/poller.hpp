#ifndef ICARUS_POLLER_HPP
#define ICARUS_POLLER_HPP

#include <map>
#include <vector>
#include <chrono>

#include "noncopyable.hpp"

#ifdef USE_EPOLL
struct epoll_event;
#else
struct pollfd;
#endif

namespace icarus
{
class EventLoop;
class Channel;

class Poller : noncopyable
{
  public:
    using ChannelList = std::vector<Channel *>;

    Poller(EventLoop *loop);

    ~Poller();

    std::chrono::system_clock::time_point poll(int timeout_ms, ChannelList *active_channels);

    void update_channel(Channel *channel);
    void remove_channel(Channel *channel);

    void assert_in_loop_thread();

  private:
    void fill_active_channels(int num_events, ChannelList *active_channels) const;

#ifdef USE_EPOLL
    using EpollEventList = std::vector<epoll_event>;
#else
    using PollFdList = std::vector<pollfd>;
#endif

    using ChannelMap = std::map<int, Channel *>;

    EventLoop *owner_loop_;
    ChannelMap channels_;
#ifdef USE_EPOLL
    int epoll_fd_;
    EpollEventList epoll_events_;
#else
    PollFdList pollfds_;
#endif
};
}
#endif // ICARUS_POLLER_HPP
