#if !defined(MUDUO_TIMER_QUEUE_H)
#define MUDUO_TIMER_QUEUE_H

#include <muduo/TimerType.h>
#include <chrono>
#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>

namespace muduo {
using namespace detail;

class EventLoop;    // forward declaration
class Watcher;      // forward declaration
class Timer;        // forward declaration

class TimerQueue {
    // non-copyable & non-moveable
    TimerQueue(const TimerQueue&) = delete;
    TimerQueue operator=(const TimerQueue) = delete;
    
    friend class Timer;
    class TimerMinHeap;

public:
    TimerQueue(EventLoop* owner);
    ~TimerQueue() noexcept;

    /** 
     * thread-safe
    */
    TimerId_t AddTimer(const TimePoint_t& when, const Interval_t& interval, const TimeoutCb_t& cb);
    void CancelTimer(const TimerId_t id);
    
    /**
     * return the loop instance whos own the current channel
    */
    EventLoop* Owner() { return owner_; }
    const EventLoop* Owner() const { return owner_; }

    void HandleExpiredTimers();

private:
    void AddTimerInLoop(std::unique_ptr<Timer>& t_p);
    void CancelTimerInLoop(const TimerId_t id);
    void ResetTimerfd();
    using ExpiredTimersList_t = std::vector<std::unique_ptr<Timer>>; 
    ExpiredTimersList_t GetExpiredTimers();

private:
    EventLoop* const owner_;
    std::unique_ptr<Watcher> watcher_;
    std::unique_ptr<TimerMinHeap> heap_;

    TimerId_t nextTimerId_;
    TimePoint_t latestTime_;
};

} // namespace muduo 

#endif // MUDUO_TIMER_QUEUE_H
