#ifndef ICARUS_POLLER_HPP
#define ICARUS_POLLER_HPP

#include <map>
#include <vector>

#include "noncopyable.hpp"

struct pollfd;

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

    // Timestamp poll(int timeout_ms, ChannelList *active_channels);

    void update_channel(Channel *channel);

    void assert_in_loop_thread();

  private:
    void fill_active_channels(int num_events, ChannelList *active_channels) const;

    using PollFdList = std::vector<pollfd>;
    using ChannelMap = std::map<int, Channel *>;

    EventLoop *owner_loop_;
    PollFdList pollfds_;
    ChannelMap channels_;
};
}
#endif // ICARUS_POLLER_HPP
