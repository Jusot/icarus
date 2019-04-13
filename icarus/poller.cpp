#include <poll.h>
#include <cassert>

#include "eventloop.hpp"
#include "channel.hpp"
#include "poller.hpp"

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

std::chrono::system_clock::time_point Poller::poll(int timeout_ms, ChannelList *active_channels)
{
    auto num_events = ::poll(&pollfds_[0], pollfds_.size(), timeout_ms);
    auto now = std::chrono::system_clock::now();

    if (num_events > 0)
    {
        fill_active_channels(num_events, active_channels);
    }
    else if (num_events < 0)
    {
        abort();
    }

    return now;
}

void Poller::update_channel(Channel *channel)
{
    assert_in_loop_thread();
    if (channel->index() < 0)
    {
        // a new one, add to pollfds_
        assert(channels_.find(channel->fd()) == channels_.end());
        pollfds_.push_back({
            channel->fd(),
            channel->events(),
            0 // revents
        });
        int idx = static_cast<int>(pollfds_.size()) - 1;
        channel->set_index(idx);
        channels_[channel->fd()] = channel;
    }
    else
    {
        // update existing one
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channels_[channel->fd()] == channel);
        int idx = channel->index();
        assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
        auto &pfd = pollfds_[idx];
        assert(pfd.fd == channel->fd() || pfd.fd == -1);
        pfd.events = channel->events();
        pfd.revents = 0;
        if (channel->is_none_event())
        {
            // ignore this pollfd
            pfd.fd = -1;
        }
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