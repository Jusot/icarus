#include <cassert>

#include <utility>

#include <poll.h>

#include "eventloop.hpp"
#include "channel.hpp"
#include "poller.hpp"

using namespace icarus;

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
    int num_events = ::poll(pollfds_.data(), pollfds_.size(), timeout_ms);
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
        assert(!channels_.count(channel->fd()));
        pollfds_.push_back({
            channel->is_none_event() ? -channel->fd() - 1 : channel->fd(),
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
        assert(channels_.count(channel->fd()));
        assert(channels_[channel->fd()] == channel);
        int idx = channel->index();
        assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
        auto &pfd = pollfds_[idx];
        assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd() - 1);
        pfd.fd = channel->is_none_event() ? -channel->fd() - 1 : channel->fd();
        pfd.events = channel->events();
        pfd.revents = 0;
    }
}

void Poller::remove_channel(Channel *channel)
{
    assert_in_loop_thread();
    assert(channels_.count(channel->fd()));
    assert(channels_[channel->fd()] == channel);
    assert(channel->is_none_event());
    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
    auto &pfd = pollfds_[idx];
    assert(pfd.fd == -channel->fd() - 1 && pfd.events == channel->events());
    auto n = channels_.erase(channel->fd());
    assert(n == 1);
    if (idx == static_cast<int>(pollfds_.size()) - 1)
    {
        pollfds_.pop_back();
    }
    else
    {
        int channel_at_end = pollfds_.back().fd;
        std::swap(pollfds_[idx], pollfds_.back());
        if (channel_at_end < 0)
        {
            channel_at_end = -channel_at_end - 1;
        }
        channels_[channel_at_end]->set_index(idx);
        pollfds_.pop_back();
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