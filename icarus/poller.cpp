#include "icarus.hpp"

namespace icarus
{
Poller::Poller(EventLoop *loop)
  : owner_loop_(loop)
{
    // ...
}

Poller::~Poller()
{
    // ...
}

// Timestamp Poller::poll(int timeout_ms, ChannelList *active_channels);

void Poller::update_channel(Channel *channel)
{
    assert_in_loop_thread();
    if (channel->index() < 0)
    {
        // a new one, add to pollfds_
    }
    else 
    {
        // update existing one
    }
}

void Poller::fill_active_channels(int num_events, ChannelList *active_channels) const
{
    for (auto pfd = pollfds_.begin(); 
        pfd != pollfds_.end() && num_events > 0; ++pfd)
    {
        if (pfd->revents > 0)
        {
            --num_events;
            auto ch = channels_.find(pfd->fd);
            assert(ch != channels_.end());
            auto channel = ch->second;
            assert(channel->fd() == pfd->fd);
            channel->set_revents(pfd->revents);
            // pfd->revents = 0;
            active_channels->push_back(channel);
        }
    }
}

void Poller::assert_in_loop_thread()
{
    owner_loop_->assert_in_loop_thread();
}
}