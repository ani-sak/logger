#pragma once

#include <atomic>
#include <cstddef>
#include <mutex>
#include <string>

// Thread safe ringbuffer
class RingBuffer {
public:
    RingBuffer(size_t buffer_size);
    RingBuffer(RingBuffer &&) = delete;
    RingBuffer(const RingBuffer &) = delete;
    RingBuffer &operator=(RingBuffer &&) = delete;
    RingBuffer &operator=(const RingBuffer &) = delete;
    ~RingBuffer();

    struct Result {
        std::string res;
        bool valid;
    };

    bool push(const std::string& s);
    Result pop();

private:
    std::atomic<size_t> head;
    std::atomic<size_t> tail;

    const std::size_t buffer_size;
    std::string *buf;

    std::mutex push_mtx;
    std::mutex pop_mtx;
};
