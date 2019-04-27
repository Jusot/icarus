#include <cassert>
#include <cstdint>
#include <cstdlib>

#include <functional>

#include <poll.h>
#include <unistd.h>
#include <sys/eventfd.h>

#include "poller.hpp"
#include "channel.hpp"
#include "eventloop.hpp"

using namespace icarus;

namespace
{
thread_local EventLoop *t_loop_in_this_thread = nullptr;

constexpr int kPollTimeMs = 10000;

static int create_eventfd()
{
    int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (fd < 0)
    {
        abort();
    }
    return fd;
}
} // namespace

EventLoop::EventLoop()
  : looping_(false),
    quit_(false),
    calling_pending_functors_(false),
    thread_id_(std::this_thread::get_id()),
    poller_(std::make_unique<Poller>(this)),
    wakeup_fd_(create_eventfd()),
    wakeup_channel_(std::make_unique<Channel>(this, wakeup_fd_))
{
    if (t_loop_in_this_thread)
    {
        abort();
    }
    else
    {
        t_loop_in_this_thread = this;
    }
    wakeup_channel_->set_read_callback(std::bind(&EventLoop::handle_read, this));
    wakeup_channel_->enable_reading();
}

EventLoop::~EventLoop()
{
    wakeup_channel_->disable_all();
    wakeup_channel_->remove();
    ::close(wakeup_fd_);
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
        do_pending_functors();
    }

    looping_ = false;
}

void EventLoop::quit()
{
    quit_ = true;
    if (!is_in_loop_thread())
    {
        wakeup();
    }
}

void EventLoop::run_in_loop(Functor cb)
{
    if (is_in_loop_thread())
    {
        cb();
    }
    else
    {
        queue_in_loop(cb);
    }
}

void EventLoop::queue_in_loop(Functor cb)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pending_functors_.push_back(cb);
    }

    if (!is_in_loop_thread() || calling_pending_functors_)
    {
        wakeup();
    }
}

std::size_t EventLoop::queue_size() const
{
    return pending_functors_.size();
}

void EventLoop::wakeup()
{
    std::uint64_t one = 1;
    auto n = ::write(wakeup_fd_, &one, sizeof one);
    /*
    if (n != sizeof one)
    {
        // LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
    */
}

void EventLoop::update_channel(Channel *channel)
{
    assert(channel->owner_loop() == this);
    assert_in_loop_thread();
    poller_->update_channel(channel);
}

void EventLoop::remove_channel(Channel *channel)
{
    assert(channel->owner_loop() == this);
    assert_in_loop_thread();
    poller_->remove_channel(channel);
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

void EventLoop::abort_not_in_loop_thread()
{
    abort();
}

void EventLoop::handle_read()
{
    std::uint64_t one = 1;
    auto n = read(wakeup_fd_, &one, sizeof one);
    /*
    if (n != sizeof one)
    {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
    */
}

void EventLoop::do_pending_functors()
{
    std::vector<Functor> functors;
    calling_pending_functors_ = true;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pending_functors_);
    }

    for (auto &functor : functors)
    {
        functor();
    }
    calling_pending_functors_ = false;
}
