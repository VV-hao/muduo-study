#if !defined(MUDUO_EVENTLOOP_THREAD_H)
#define MUDUO_EVENTLOOP_THREAD_H
#include <muduo/base/allocator/Allocatable.h>
#include <muduo/Callbacks.h>
#include <mutex>
#include <memory>
#include <atomic>
#include <thread>
#include <condition_variable>

namespace muduo {

#ifdef MUDUO_USE_MEMPOOL
class EventLoopThread : public base::detail::Allocatable {
#else
class EventLoopThread {
#endif
    // non-copyable & non-moveable
    EventLoopThread(const EventLoopThread&) = delete;
    EventLoopThread operator=(const EventLoopThread&) = delete;
public:
    EventLoopThread(const IoThreadInitCallback_t& cb = nullptr, const std::string& name = std::string());
    ~EventLoopThread() noexcept;

    /* @note Only Can be called once */
    EventLoop* Run();

private:
    void ThreadFunc();

private:
    EventLoop* loop_;   // EventLoop instance of IO-thread
    std::string name_;
    IoThreadInitCallback_t initCb_;
    std::unique_ptr<std::thread> IoThread_;              // current IO-thread
    bool isExit_;                       // The state dictates whether IO thread exits 
    /* for sync operations on loop_ */
    std::mutex mtx_;
    std::condition_variable cv_;
};


} // namespace muduo 


#endif // MUDUO_EVENTLOOP_THREAD_H
