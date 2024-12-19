#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <mutex>

namespace Ringbuffer {

template <typename T> class RingBuffer {
public:
    RingBuffer(size_t buffer_size);
    RingBuffer(RingBuffer&&) = delete;
    RingBuffer(const RingBuffer&) = delete;
    auto operator=(RingBuffer&&) -> RingBuffer& = delete;
    auto operator=(const RingBuffer&) -> RingBuffer& = delete;
    ~RingBuffer();

    class Result {
    public:
        Result(const T& res, bool valid = false) : res(res), valid(valid) {}
        auto data() const -> T {
            if (!valid) {
                return {};
            }

            return res;
        }
        [[nodiscard]] auto err() const -> bool { return !valid; }

    private:
        T res;
        const bool valid;
    };

    auto try_push(const T& data) -> bool;
    auto try_push(const T& data,
                  const std::chrono::microseconds& duration) -> bool;
    void push(const T& data);
    auto try_pop() -> Result;
    auto try_pop(const std::chrono::microseconds& duration) -> Result;
    auto pop() -> T;

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

    void push_impl(const T& data);
    auto pop_impl() -> T;
};

//===----------------------------------------------------------------------===//

template <typename T>
RingBuffer<T>::RingBuffer(size_t buffer_size)
    : head(0), tail(0), num_entries(0), buffer_size(buffer_size),
      buf(new T[buffer_size]) {}

template <typename T> RingBuffer<T>::~RingBuffer() { delete[] buf; }

//===----------------------------------------------------------------------===//

template <typename T> void RingBuffer<T>::push_impl(const T& data) {
    buf[tail] = data;
    tail = (tail + 1) % buffer_size;
    num_entries++;
    cv_buf_not_empty.notify_one();
}

template <typename T> auto RingBuffer<T>::try_push(const T& data) -> bool {
    std::lock_guard<std::mutex> lock(push_mtx);

    if (num_entries >= buffer_size) {
        return false;
    }

    push_impl(data);
    return true;
}

template <typename T>
auto RingBuffer<T>::try_push(
    const T& data, const std::chrono::microseconds& duration) -> bool {
    std::unique_lock<std::mutex> lock(push_mtx);

    if (num_entries >= buffer_size) {
        if (!cv_buf_not_full.wait_for(
                lock, duration, [this] { return num_entries < buffer_size; })) {
            return false;
        }
    }

    push_impl(data);
    return true;
}

template <typename T> void RingBuffer<T>::push(const T& data) {
    std::unique_lock<std::mutex> lock(push_mtx);

    if (num_entries >= buffer_size) {
        cv_buf_not_full.wait(lock,
                             [this] { return num_entries < buffer_size; });
    }

    push_impl(data);
}

template <typename T> auto RingBuffer<T>::pop_impl() -> T {
    std::size_t old_head = head;
    head = (head + 1) % buffer_size;
    num_entries--;
    cv_buf_not_full.notify_one();
    return buf[old_head];
}

template <typename T> auto RingBuffer<T>::try_pop() -> RingBuffer<T>::Result {
    std::lock_guard<std::mutex> lock(pop_mtx);

    if (num_entries <= 0) {
        return {T{}, false};
    }

    const T retval = pop_impl();
    return {retval, true};
}

template <typename T>
auto RingBuffer<T>::try_pop(const std::chrono::microseconds& duration)
    -> RingBuffer<T>::Result {
    std::unique_lock<std::mutex> lock(pop_mtx);

    if (num_entries <= 0) {
        if (!cv_buf_not_empty.wait_for(lock, duration,
                                       [this] { return num_entries > 0; })) {
            return {T{}, false};
        }
    }

    const T retval = pop_impl();
    return {retval, true};
}

template <typename T> auto RingBuffer<T>::pop() -> T {
    std::unique_lock<std::mutex> lock(pop_mtx);

    if (num_entries == 0) {
        cv_buf_not_empty.wait(lock, [this] { return num_entries > 0; });
    }

    return pop_impl();
}

} // namespace Ringbuffer
