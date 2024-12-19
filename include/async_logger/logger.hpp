#pragma once

#include <cstddef>
#include <memory>
#include <string>

namespace Logger {

enum class LogLevel { Debug, Warn, Error };

constexpr std::size_t default_queue_size = 8192;

class Logger {
public:
    Logger() = default;
    Logger(Logger&&) = delete;
    Logger(const Logger&) = delete;
    Logger& operator=(Logger&&) = delete;
    Logger& operator=(const Logger&) = delete;
    virtual ~Logger() = default;

    // Threadsafe log that minimizes blocking client thread
    virtual void log(LogLevel loglevel, const std::string& logmsg) = 0;
};

std::shared_ptr<Logger>
ConsoleLogger(std::size_t queue_size = default_queue_size);

std::shared_ptr<Logger> FileLogger(const std::string& logfile,
                                   std::size_t queue_size = default_queue_size);
} // namespace Logger
