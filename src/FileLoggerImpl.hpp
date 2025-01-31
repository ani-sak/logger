#pragma once

#include "async_logger/logger.hpp"
#include "fmt/os.h"
#include "ringbuffer.hpp"

#include <cstddef>
#include <string>
#include <thread>

namespace Logger {

class FileLoggerImpl : public Logger {
public:
    template <typename T> using RB = Ringbuffer::RingBuffer<T>;

    FileLoggerImpl(std::size_t queue_size, const std::string& logfile,
                   LogStrategy log_strategy);
    FileLoggerImpl(FileLoggerImpl&&) = delete;
    FileLoggerImpl(const FileLoggerImpl&) = delete;
    auto operator=(FileLoggerImpl&&) -> FileLoggerImpl& = delete;
    auto operator=(const FileLoggerImpl&) -> FileLoggerImpl& = delete;
    ~FileLoggerImpl() override;

    void log(LogLevel loglevel, const std::string& logmsg) override;

private:
    std::unique_ptr<RB<std::string>> buffer;
    fmt::ostream file;
    bool stop_log_thread{};
    LogStrategy log_strategy;
    void store_logs();
    std::thread log_thread;
};

} // namespace Logger
