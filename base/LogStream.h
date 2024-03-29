#if !defined(MUDUO_BASE_LOG_STREAM_H)
#define MUDUO_BASE_LOG_STREAM_H
#include <string>
#include <cassert>
#include <cstdint>
#include <cstring>

namespace muduo {
namespace base {

namespace detail {

const size_t kLargeBuffer = 4000*1024; 
const size_t kSmallBuffer = 4000;

template <int SIZE>
class FixedBuffer {
    using size_t = std::size_t;
    
public:
    FixedBuffer()
        : cur_(data_)
        { }
    
    FixedBuffer(const FixedBuffer& buf) 
        : FixedBuffer()
    {
        std::memcpy(data_, buf.data_, buf.GetLength());
        cur_ += buf.GetLength();
    }

    const char* Data() const
    { return data_; }

    void Append(const void* data, size_t len) {
        if (Avail() > len) {
            std::memcpy(Current(), data, len);
            AddLength(len);
        }
    }

    const char* Current() const 
    { return cur_; }

    size_t GetLength() const
    { return cur_ - data_; }

    size_t Avail() const
    { return End() - cur_; } 

    void Reset() 
    {
        std::memset(data_, 0, sizeof data_);
        cur_ = data_;
    }

    std::string ToString() const 
    { return std::string(data_, GetLength()); }

private:
    char* Current()
    { return cur_; }

    const char* End() const
    { return data_ + sizeof data_; }

    void AddLength(size_t len)
    {
        // assert(len < Avail());
        cur_ += len;
    }

private:
    char data_[SIZE] {0};
    char* cur_;
};

} // namespace detail 

class LogStream {
    using self = LogStream;

    /* non-copyable & non-moveable */
    LogStream(const LogStream&) = delete;
    LogStream& operator=(const LogStream&) = delete;

public:
#ifdef MUDUO_LOG_LARGE_BUFFER
    using Buffer = detail::FixedBuffer<kLargeBuffer>;
#else
    using Buffer = detail::FixedBuffer<detail::kSmallBuffer>;
#endif

    LogStream()
        : buf_(Buffer())
        { }

    void Append(const void* data, size_t len) 
    { buf_.Append(data, len); }

    const Buffer& GetInternalBuf() const
    { return buf_; }

    void ResetBuffer()
    { buf_.Reset(); }

    template <typename T>
    self& operator<<(T val) {
        std::string formated = std::to_string(val);
        return *this << formated; 
    }

    /// For pointers, except (const)void* and (const)char*
    template <typename T>
    self& operator<<(T* ptr) {
        // invoke non-template member func: LogStream::operator<<(const void* ptr)
        return *this << static_cast<void*>(ptr);        
    }

    /* Member functions take precedence over template functions */
    self& operator<<(const std::string& str) {
        Append(str.c_str(), str.size());
        return *this;
    }

    self& operator<<(char c) {
        Append(&c, sizeof c);
        return *this;
    }

    self& operator<<(const void* ptr);

    self& operator<<(void* ptr)
    { return *this << const_cast<const void*>(ptr); }

    self& operator<<(const char* str);

    self& operator<<(char* str)
    { return *this << const_cast<const char*>(str); }

private:
    Buffer buf_;
    
};

template<>
LogStream::self& LogStream::operator<< <bool>(bool boolean);

template<>
LogStream::self& LogStream::operator<< <double>(double d);

template<>
LogStream::self& LogStream::operator<< <float>(float f);

} // namespace base 
} // namespace muduo 

#endif // MUDUO_BASE_LOG_STREAM_H
