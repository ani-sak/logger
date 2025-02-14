#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <type_traits>
#include <utility>

namespace Ringbuffer {

template <typename T>
class RingBuffer {
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
        auto data() const -> T { return (valid ? res : T{}); }
        [[nodiscard]] auto err() const -> bool { return !valid; }

    private:
        T res;
        bool valid;
    };

    template <typename U> // Trigger type deduction for universal refs
    auto try_push(U&& data) -> bool;

    template <typename U>
    auto try_push(U&& data, const std::chrono::microseconds& duration) -> bool;

    template <typename U>
    void push(U&& data);

    auto try_pop() -> Result;

    auto try_pop(const std::chrono::microseconds& duration) -> Result;

    auto pop() -> T;

private:
    std::atomic<std::size_t> head;
    std::atomic<std::size_t> tail;
    std::atomic<std::size_t> num_entries;

    const std::size_t buffer_size;
    T* buf;

    std::mutex mtx;

    std::condition_variable cv_buf_not_full;
    std::condition_variable cv_buf_not_empty;

    template <typename U>
    void push_impl(U&& data);

    auto pop_impl() -> T;
};

//===----------------------------------------------------------------------===//

template <typename T>
RingBuffer<T>::RingBuffer(size_t buffer_size)
    : head(0), tail(0), num_entries(0), buffer_size(buffer_size),
      buf(new T[buffer_size]) {}

template <typename T>
RingBuffer<T>::~RingBuffer() {
    delete[] buf;
}

//===----------------------------------------------------------------------===//

template <typename T>
template <typename U>
void RingBuffer<T>::push_impl(U&& data) {
    static_assert(std::is_assignable_v<T&, U>,
                  "Cannot push incompatible type U to Ringbuffer of type T");

    buf[tail] = std::forward<U>(data);
    tail = (tail + 1) % buffer_size;
    num_entries++;
    cv_buf_not_empty.notify_one();
}

template <typename T>
template <typename U>
auto RingBuffer<T>::try_push(U&& data) -> bool {
    static_assert(std::is_assignable_v<T&, U>,
                  "Cannot push incompatible type U to Ringbuffer of type T");

    std::unique_lock<std::mutex> lock(mtx);

    if (num_entries >= buffer_size) {
        return false;
    }

    push_impl(std::forward<U>(data));
    return true;
}

template <typename T>
template <typename U>
auto RingBuffer<T>::try_push(U&& data,
                             const std::chrono::microseconds& duration)
    -> bool {
    static_assert(std::is_assignable_v<T&, U>,
                  "Cannot push incompatible type U to Ringbuffer of type T");

    std::unique_lock<std::mutex> lock(mtx);

    if (num_entries >= buffer_size) {
        if (!cv_buf_not_full.wait_for(
                lock, duration, [this] { return num_entries < buffer_size; })) {
            return false;
        }
    }

    push_impl(std::forward<U>(data));
    return true;
}

template <typename T>
template <typename U>
void RingBuffer<T>::push(U&& data) {
    static_assert(std::is_assignable_v<T&, U>,
                  "Cannot push incompatible type U to Ringbuffer of type T");

    std::unique_lock<std::mutex> lock(mtx);

    if (num_entries >= buffer_size) {
        cv_buf_not_full.wait(lock,
                             [this] { return num_entries < buffer_size; });
    }

    push_impl(std::forward<U>(data));
}

template <typename T>
auto RingBuffer<T>::pop_impl() -> T {
    std::size_t old_head = head;
    head = (head + 1) % buffer_size;
    num_entries--;
    cv_buf_not_full.notify_one();
    return buf[old_head];
}

template <typename T>
auto RingBuffer<T>::try_pop() -> RingBuffer<T>::Result {
    std::unique_lock<std::mutex> lock(mtx);

    if (num_entries <= 0) {
        return {T{}, false};
    }

    return {pop_impl(), true};
}

template <typename T>
auto RingBuffer<T>::try_pop(const std::chrono::microseconds& duration)
    -> RingBuffer<T>::Result {
    std::unique_lock<std::mutex> lock(mtx);

    if (num_entries <= 0) {
        if (!cv_buf_not_empty.wait_for(lock, duration,
                                       [this] { return num_entries > 0; })) {
            return {T{}, false};
        }
    }

    return {pop_impl(), true};
}

template <typename T>
auto RingBuffer<T>::pop() -> T {
    std::unique_lock<std::mutex> lock(mtx);

    if (num_entries <= 0) {
        cv_buf_not_empty.wait(lock, [this] { return num_entries > 0; });
    }

    return pop_impl();
}

} // namespace Ringbuffer
