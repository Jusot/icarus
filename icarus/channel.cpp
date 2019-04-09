#include "icarus.hpp"

namespace icarus
{
Channel::Channel(EventLoop* loop, int fd)
  : loop_(loop),
    fd_(fd),
    events_(0),
    revents_(0),
    index_(-1)
{
    // ...
}

void Channel::handleEvent()
{
    if (revents_ & POLLNVAL)
    {
        // ...
    }

    if (revents_ & (POLLERR | POLLNVAL))
    {
        if (error_callback_)
        {
            error_callback_();
        }
    }
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
    {
        if (read_callback_)
        {
            read_callback_();
        }
    }
    if (revents_ & POLLOUT)
    {
        if (write_callback_)
        {
            write_callback_();
        }
    }
}

void Channel::update()
{
    // loop_->update_channel(this);
}
}