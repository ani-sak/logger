#pragma once

#include "async_logger/logger.hpp"
#include "ringbuffer.hpp"
#include <thread>

namespace Logger {

class ConsoleLoggerImpl : public Logger {
public:
    template <typename T> using RB = Ringbuffer::RingBuffer<T>;

    ConsoleLoggerImpl(std::size_t queue_size);
    ConsoleLoggerImpl(ConsoleLoggerImpl&&) = delete;
    ConsoleLoggerImpl(const ConsoleLoggerImpl&) = delete;
    auto operator=(ConsoleLoggerImpl&&) -> ConsoleLoggerImpl& = delete;
    auto operator=(const ConsoleLoggerImpl&) -> ConsoleLoggerImpl& = delete;
    ~ConsoleLoggerImpl() override;

    void log(LogLevel loglevel, const std::string& logmsg) override;

private:
    std::unique_ptr<RB<std::string>> buffer;
    bool stop_log_thread{};
    void store_logs();
    std::thread log_thread;
};

} // namespace Logger
