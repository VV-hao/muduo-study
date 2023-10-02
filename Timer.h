#if !defined(MUDUO_TIMER_H)
#define MUDUO_TIMER_H

#include <TimerQueue.h>
#include <TimerType.h>
#include <Channel.h>
#include <logger.h>
#include <Timer.h>
#include <memory>
#include <sys/timerfd.h>

namespace muduo {
using namespace detail;

class Timer {
    // noncopyable & nonmoveable
    Timer(const Timer&) = delete;
    Timer(Timer&&) = delete;
    friend TimerQueue::ExpiredTimersList_t TimerQueue::GetExpiredTimers();

public:
    Timer(const TimePoint_t& timeout, const Interval_t& interval_ms, const TimeoutCb_t& cb, const TimerId_t id)
        : cb_(cb)
        , expiration_(timeout)
        , interval_(interval_ms)
        , id_(id)
        { }

    void Run() const { cb_(); }
    TimePoint_t ExpirationTime() const { return expiration_; }
    Interval_t Interval() const { return interval_; }
    TimerId_t GetId() const { return id_; }
    bool Repeat() const { return interval_ != Interval_t::zero(); }

private:
    TimeoutCb_t cb_;
    TimePoint_t expiration_;
    Interval_t interval_;
    TimerId_t id_;
};

/*************************************************************************************************/

class Watcher {
    // non-copyable & non-moveable
    Watcher(const Watcher&) = delete;
    Watcher operator=(const Watcher&) = delete;

public:
    Watcher(TimerQueue* owner); 
    ~Watcher() noexcept;
    
    void HandleExpiredTimers();
    int FileDescriptor() const
    { return watcherChannel_->FileDescriptor(); }

    void SetTimerfd(struct itimerspec* old_t, struct itimerspec* new_t) const {
        int ret = ::timerfd_settime(FileDescriptor(), TFD_TIMER_ABSTIME, new_t, old_t);
        if (ret != 0) {
            // LOG-LEVEL: ERROR
            std::cerr << "Failed to set timerfd, detail: " << muduo::strerror_thread_safe(errno) << std::endl;
            // wether need abort() ?
        }
    }

private:
    void ReadTimerfd() const;

private:
    TimerQueue* const owner_;   // 聚合
    std::unique_ptr<Channel> watcherChannel_;
};

} // namespace muduo 



#endif // MUDUO_TIMER_H