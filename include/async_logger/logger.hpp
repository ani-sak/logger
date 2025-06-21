#ifndef ASYNC_LOGGER_LOGGER_HPP
#define ASYNC_LOGGER_LOGGER_HPP

#include <cstddef>
#include <memory>
#include <string>

namespace Logger {

enum class LogLevel { Debug, Warn, Error };

constexpr std::size_t default_queue_size = 8192;

enum class LogStrategy { Blocking, Immediate };

class Logger {
public:
    Logger() = default;
    Logger(Logger&&) = delete;
    Logger(const Logger&) = delete;
    auto operator=(Logger&&) -> Logger& = delete;
    auto operator=(const Logger&) -> Logger& = delete;
    virtual ~Logger() = default;

    // Threadsafe log that minimizes blocking client thread
    virtual void log(LogLevel loglevel, const std::string& logmsg) = 0;
};

auto ConsoleLogger(std::size_t queue_size = default_queue_size,
                   LogStrategy log_strategy = LogStrategy::Immediate)
    -> std::shared_ptr<Logger>;

auto FileLogger(const std::string& logfile,
                std::size_t queue_size = default_queue_size,
                LogStrategy log_strategy = LogStrategy::Immediate)
    -> std::shared_ptr<Logger>;

} // namespace Logger


namespace AsyncLogger {

constexpr std::size_t default_buffer_size = 64;
constexpr std::size_t default_entry_size = 1024;

enum class LogLevel { Debug, Warn, Error };

class Buffer; // Forward Declaration
using SharedPtrBuffer = std::shared_ptr<Buffer>;

auto create_buffer(std::size_t buffer_size = default_buffer_size,
                   std::size_t entry_size = default_entry_size) -> SharedPtrBuffer;

auto create_buffer(const std::string& logfile,
                   std::size_t buffer_size = default_buffer_size,
                   std::size_t entry_size = default_entry_size) -> SharedPtrBuffer;

auto log(SharedPtrBuffer buffer, LogLevel loglevel, const std::string& logmsg) -> bool;

auto flush(SharedPtrBuffer buffer) -> bool;
} // namespace AsyncLogger

#endif // !ASYNC_LOGGER_LOGGER_HPP
