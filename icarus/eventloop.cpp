#include "icarus.hpp"

#include <cstdlib>

namespace icarus
{
thread_local EventLoop *t_loop_in_this_thread = nullptr;

EventLoop::EventLoop()
  : looping_(false),
    thread_id_(std::this_thread::get_id())
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

    ::poll(nullptr, 0, 5 * 1000);

    looping_ = false;
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
