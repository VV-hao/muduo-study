#include <EventLoop.h>
#include <Channel.h>
#include <poll.h>
#include <cassert>

using namespace muduo;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI | POLLRDHUP;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* owner, int fd)
    : owner_(owner)
    , fd_(fd)
    , events_(kNoneEvent)
    , revents_(kNoneEvent)
    , index_(-1)
    , logHup_(true)
    , tie_()
    , tied_(false)
    , eventHandling_(false)
    { }

void Channel::HandleEvents(ReceiveTimePoint_t receiveTime) {
    if (tied_) {
        std::shared_ptr<void> guard(tie_.lock());
        if (guard) {
            HandleEventsWithGuard(receiveTime);
        }
    } else {
        HandleEventsWithGuard(receiveTime);
    }
}

void Channel::HandleEventsWithGuard(ReceiveTimePoint_t receiveTime) {
    assert(!eventHandling_);
    eventHandling_ = true;

    // 当本端接收到对端发来的FIN数据包时，本端的RCV_SHUTDOWN被设置
    // 当本端发送FIN数据包并收到了对端的ACK包时，本端的SEND_SHUTDOWN被设置
    // POLLHUP: When "(RCV_SHUTDOWN|SEND_SHUTDOWN) || state == TCP_CLOSE"
    // for example: 当socket的读和写端都被关闭，但却没有关闭该socket，则poll时POLLHUP事件会被返回
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
        if (logHup_) {
            std::clog << "POLLHUP" << std::endl;
        }
        if (closeCb_) {
            closeCb_();
        }
    }

    // POLLHUP: When the file descriptor is closed or never opened
    if (revents_ & POLLNVAL) {
        std::clog << "POLLNVAL, fd=" << FileDescriptor() << std::endl;
    }

    // POLLERR: When the fd has a async-error
    if (revents_ & (POLLERR | POLLNVAL)) {
        if (errorCb_) { errorCb_(); }
    }

    // POLLRDHUP: When RCV_SHUTDOWN is set(即对端关闭了fd或只关闭了写端)
    if (revents_ & (POLLIN | POLLRDHUP | POLLPRI)) {
        if (readCb_) { readCb_(receiveTime); }
    }

    if (revents_ & POLLOUT) {
        if (writeCb_) { writeCb_(); }
    }

    eventHandling_ = false;
}

void Channel::Update() {
    owner_->UpdateChannel(this);
}

void Channel::Remove() {
    assert(IsNoneEvent());
    owner_->RemoveChannel(this);
}