#include "ringbuffer.hpp"

#include <cstddef>
#include <mutex>

RingBuffer::RingBuffer(size_t buffer_size)
    : head(0), tail(0), buffer_size(buffer_size), buf(nullptr), push_mtx(),
      pop_mtx() {
    buf = new std::string[buffer_size];
}

RingBuffer::~RingBuffer() { delete[] buf; }

namespace {
enum class BufferUtilStatus { Empty, Partial, Full };

BufferUtilStatus compute(std::size_t head, std::size_t tail,
                         std::size_t buffer_size) {
    if (head == tail) {
        return BufferUtilStatus::Empty;
    }

    if ((tail + 1) % buffer_size == head) {
        return BufferUtilStatus::Full;
    }

    return BufferUtilStatus::Partial;
}
} // namespace

bool RingBuffer::push(const std::string &s) {
    switch (compute(head, tail, buffer_size)) {
    case BufferUtilStatus::Empty:
    case BufferUtilStatus::Partial: {
        std::lock_guard<std::mutex> lock(push_mtx);
        buf[tail] = s;
        tail = (tail + 1) % buffer_size;
        return true;
    }

    // Return if buffer is full
    // Avoid waiting for space to free up
    // Avoid blocking main thread
    case BufferUtilStatus::Full: {
        return false;
    }
    default:
        return false;
    }
}

RingBuffer::Result RingBuffer::pop() {
    switch (compute(head, tail, buffer_size)) {
    case BufferUtilStatus::Empty: {
        return {"", false};
    }
    case BufferUtilStatus::Partial:
    case BufferUtilStatus::Full: {
        std::lock_guard<std::mutex> lock(pop_mtx);
        std::size_t old_head = head;
        head = (head + 1) % buffer_size;
        return {buf[old_head], true};
    }
    default:
        return {"", false};
    }
}
