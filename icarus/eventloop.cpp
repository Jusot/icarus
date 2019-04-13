#include <poll.h>
#include <cassert>
#include <cstdlib>

#include "poller.hpp"
#include "channel.hpp"
#include "eventloop.hpp"

using namespace icarus;

namespace
{
thread_local EventLoop *t_loop_in_this_thread = nullptr;

constexpr int kPollTimeMs = 10000;

EventLoop::EventLoop()
  : looping_(false),
    quit_(false),
    thread_id_(std::this_thread::get_id()),
    poller_(std::make_unique<Poller>(this))
{
    if (t_loop_in_this_thread)
    {
        abort();
    }
    else
    {
        t_loop_in_this_thread = this;
    }
}

EventLoop::~EventLoop()
{
    assert(!looping_);
    t_loop_in_this_thread = nullptr;
}

void EventLoop::loop()
{
    assert(!looping_);
    assert_in_loop_thread();
    looping_ = true;
    quit_ = false;

    while (!quit_)
    {
        active_channels_.clear();
        poller_->poll(kPollTimeMs, &active_channels_);
        for (auto &channel : active_channels_)
        {
            channel->handle_event();
        }
    }

    looping_ = false;
}

void EventLoop::quit()
{
    quit_ = true;
    // wakeup();
}

void EventLoop::update_channel(Channel *channel)
{
    assert(channel->owner_loop() == this);
    assert_in_loop_thread();
    poller_->update_channel(channel);
}

void EventLoop::assert_in_loop_thread()
{
    if (!is_in_loop_thread())
    {
        abort_not_in_loop_thread();
    }
}

bool EventLoop::is_in_loop_thread() const
{
    return thread_id_ == std::this_thread::get_id();
}

EventLoop *EventLoop::get_event_loop_of_current_thread()
{
    return t_loop_in_this_thread;
}
} // namespace icarus
