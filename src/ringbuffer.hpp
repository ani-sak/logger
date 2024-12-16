#pragma once

#include <atomic>
#include <cstddef>
#include <mutex>

// Thread safe ringbuffer
template<typename T>
class RingBuffer {
public:
    RingBuffer(size_t buffer_size);
    RingBuffer(RingBuffer &&) = delete;
    RingBuffer(const RingBuffer &) = delete;
    RingBuffer &operator=(RingBuffer &&) = delete;
    RingBuffer &operator=(const RingBuffer &) = delete;
    ~RingBuffer();

    class Result {
    public:
        Result(const T &r, bool valid = false) : res(r), valid(valid) {}
        T data() {
            if (!valid) {
                return {};
            }

            return res;
        }
        bool err() { return !valid; }

    private:
        T res;
        bool valid;
    };

    bool push(const T& data); // Fails if queue full
    // void push_blocking (const T& data); // Blocks if queue full
    Result pop();
    // void pop_blocking (const T& data); // Blocks if queue empty

private:
    std::atomic<size_t> head;
    std::atomic<size_t> tail;

    const std::size_t buffer_size;
    T *buf;

    std::mutex push_mtx;
    std::mutex pop_mtx;

    enum class BufferUtilStatus { Empty, Partial, Full };
    BufferUtilStatus compute();
};

//===----------------------------------------------------------------------===//

template <typename T>
RingBuffer<T>::RingBuffer(size_t buffer_size)
    : head(0), tail(0), buffer_size(buffer_size), buf(nullptr), push_mtx(),
      pop_mtx() {
    buf = new T[buffer_size];
}

template <typename T> RingBuffer<T>::~RingBuffer() { delete[] buf; }

template <typename T>
typename RingBuffer<T>::BufferUtilStatus RingBuffer<T>::compute() {
    if (head == tail) {
        return BufferUtilStatus::Empty;
    }

    if ((tail + 1) % buffer_size == head) {
        return BufferUtilStatus::Full;
    }

    return BufferUtilStatus::Partial;
}

template <typename T> bool RingBuffer<T>::push(const T& data) {
    switch (compute()) {
    case BufferUtilStatus::Empty:
    case BufferUtilStatus::Partial: {
        std::lock_guard<std::mutex> lock(push_mtx);
        buf[tail] = data;
        tail = (tail + 1) % buffer_size;
        return true;
    }
    case BufferUtilStatus::Full: {
        return false;
    }
    default:
        return false;
    }
}

template <typename T> typename RingBuffer<T>::Result RingBuffer<T>::pop() {
    switch (compute()) {
    case BufferUtilStatus::Empty: {
        return {T{}, false};
    }
    case BufferUtilStatus::Partial:
    case BufferUtilStatus::Full: {
        std::lock_guard<std::mutex> lock(pop_mtx);
        std::size_t old_head = head;
        head = (head + 1) % buffer_size;
        return {buf[old_head], true};
    }
    default:
        return {T{}, false};
    }
}
