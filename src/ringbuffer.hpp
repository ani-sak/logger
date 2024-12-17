#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <mutex>

// Thread safe ringbuffer
template <typename T> class RingBuffer {
public:
    RingBuffer(size_t buffer_size);
    RingBuffer(RingBuffer&&) = delete;
    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(RingBuffer&&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;
    ~RingBuffer();

    class Result {
    public:
        Result(const T& r, bool valid = false) : res(r), valid(valid) {}
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

    bool try_push(const T& data);
    void push(const T& data);
    Result try_pop();
    T pop();

private:
    std::atomic<size_t> head;
    std::atomic<size_t> tail;
    std::atomic<size_t> num_entries;

    const std::size_t buffer_size;
    T* buf;

    std::mutex push_mtx;
    std::mutex pop_mtx;

    std::condition_variable cv_buf_not_full;
    std::condition_variable cv_buf_not_empty;
};

//===----------------------------------------------------------------------===//

template <typename T>
RingBuffer<T>::RingBuffer(size_t buffer_size)
    : head(0), tail(0), num_entries(0), buffer_size(buffer_size), buf(nullptr),
      push_mtx(), pop_mtx() {
    buf = new T[buffer_size];
}

template <typename T> RingBuffer<T>::~RingBuffer() { delete[] buf; }

template <typename T> bool RingBuffer<T>::try_push(const T& data) {
    std::lock_guard<std::mutex> lock(push_mtx);

    if (num_entries >= buffer_size) {
        return false;
    }

    buf[tail] = data;
    tail = (tail + 1) % buffer_size;
    num_entries++;

    cv_buf_not_empty.notify_one();
    return true;
}

template <typename T> void RingBuffer<T>::push(const T& data) {
    std::unique_lock<std::mutex> lock(push_mtx);

    if (num_entries >= buffer_size) {
        cv_buf_not_full.wait(lock,
                             [this] { return num_entries < buffer_size; });
    }

    buf[tail] = data;
    tail = (tail + 1) % buffer_size;
    num_entries++;

    cv_buf_not_empty.notify_one();
    return;
}

template <typename T> typename RingBuffer<T>::Result RingBuffer<T>::try_pop() {
    std::lock_guard<std::mutex> lock(pop_mtx);

    if (num_entries <= 0) {
        return {T{}, false};
    }

    std::size_t old_head = head;
    head = (head + 1) % buffer_size;
    num_entries--;
    cv_buf_not_full.notify_one();
    return {buf[old_head], true};
}

template <typename T> T RingBuffer<T>::pop() {
    std::unique_lock<std::mutex> lock(pop_mtx);

    if (num_entries == 0) {
        cv_buf_not_empty.wait(lock, [this] { return num_entries > 0; });
    }

    std::size_t old_head = head;
    head = (head + 1) % buffer_size;
    num_entries--;
    cv_buf_not_full.notify_one();
    return buf[old_head];
}
