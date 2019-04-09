#ifndef ICARUS_EVENTLOOP_HPP
#define ICARUS_EVENTLOOP_HPP

namespace icarus
{
class EventLoop : noncopyable
{
  public:
    // default contructor
    EventLoop();

    // default deconstructor
    ~EventLoop();

    // main loop
    void loop();

    // assert current thread is in loop thread or not
    void assert_in_loop_thread();

    // checks current thread is in loop thread or not
    bool is_in_loop_thread() const;

    // returns EventLoop object's address of current thread
    static EventLoop *get_event_loop_of_current_thread();

  private:
    // abort when not in loop thread
    void abort_not_in_loop_thread();

    bool looping_;
    const std::thread::id thread_id_;
};
} // namespace icarus

#endif // ICARUS_EVENTLOOP_HPP