#include <cassert>

#include <utility>

#ifdef USE_EPOLL
#include <sys/epoll.h>
#include <unistd.h>
#else
#include <poll.h>
#endif

#include "eventloop.hpp"
#include "channel.hpp"
#include "poller.hpp"

using namespace icarus;

Poller::Poller(EventLoop *loop)
  : owner_loop_(loop)
{
#ifdef USE_EPOLL
    epoll_fd_ = ::epoll_create(10);
    if (epoll_fd_ == -1)
    {
        abort();
    }
#endif
}

Poller::~Poller()
{
#ifdef USE_EPOLL
    ::close(epoll_fd_);
#endif
}

std::chrono::system_clock::time_point Poller::poll(int timeout_ms, ChannelList *active_channels)
{
#ifdef USE_EPOLL
    int num_events = ::epoll_wait(epoll_fd_, epoll_events_.data(), epoll_events_.size(), timeout_ms);
#else
    int num_events = ::poll(pollfds_.data(), pollfds_.size(), timeout_ms);
#endif
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
#ifdef USE_EPOLL
        epoll_event event;
        event.events = channel->events();
        event.data.fd = channel->fd();
        if (::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, channel->fd(), &event) == -1)
        {
            abort();
        }
        epoll_events_.push_back(event);
        int idx = static_cast<int>(epoll_events_.size()) - 1;
#else
        pollfds_.push_back({
            channel->is_none_event() ? -channel->fd() - 1 : channel->fd(),
            channel->events(),
            0 // revents
        });
        int idx = static_cast<int>(pollfds_.size()) - 1;
#endif
        channel->set_index(idx);
        channels_[channel->fd()] = channel;
    }
    else
    {
        // update existing one
        assert(channels_.count(channel->fd()));
        assert(channels_[channel->fd()] == channel);
#ifdef USE_EPOLL
        if (channel->is_none_event())
        {
            epoll_event event;
            event.events = channel->events();
            event.data.fd = channel->fd();
            if (::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, channel->fd(), &event) == -1)
            {
                abort();
            }
        }
        else
        {
            if (::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, channel->fd(), nullptr) == -1)
            {
                abort();
            }
        }
#else
        int idx = channel->index();
        assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
        auto &pfd = pollfds_[idx];
        assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd() - 1);
        pfd.fd = channel->is_none_event() ? -channel->fd() - 1 : channel->fd();
        pfd.events = channel->events();
        pfd.revents = 0;
#endif
    }
}

void Poller::remove_channel(Channel *channel)
{
    assert_in_loop_thread();
    assert(channels_.count(channel->fd()));
    assert(channels_[channel->fd()] == channel);
    assert(channel->is_none_event());
    int idx = channel->index();
#ifdef USE_EPOLL
    assert(0 <= idx && idx < static_cast<int>(epoll_events_.size()));
    epoll_events_.pop_back();
#else
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
    auto &pfd = pollfds_[idx];
    assert(pfd.fd == -channel->fd() - 1 && pfd.events == channel->events());
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
#endif
    auto n = channels_.erase(channel->fd());
    assert(n == 1);
}

void Poller::fill_active_channels(int num_events, ChannelList *active_channels) const
{
#ifdef USE_EPOLL
    for (int i = 0; i < num_events; ++i)
#else
    for (auto pfd = pollfds_.begin();
        pfd != pollfds_.end() && num_events > 0; ++pfd)
#endif
    {
#ifdef USE_EPOLL
            int fd = epoll_events_[i].data.fd;
            int revent = epoll_events_[i].events;
#else
        if (pfd->revents > 0)
        {
            --num_events;
            int fd = pfd->fd;
            int revent = pfd->revents;
#endif
            auto ch = channels_.find(fd);
            assert(ch != channels_.end());
            auto channel = ch->second;
            assert(channel->fd() == fd);
            channel->set_revents(revent);
            // pfd->revents = 0;
            active_channels->push_back(channel);
#ifndef USE_EPOLL
        }
#endif
    }

}

void Poller::assert_in_loop_thread()
{
    owner_loop_->assert_in_loop_thread();
}
