#include "async_logger/logger.hpp"
#include "fmt/base.h"
#include "fmt/chrono.h"
#include "fmt/color.h"
#include "fmt/format.h"
#include "fmt/os.h"
#include "ringbuffer.hpp"

#include <chrono>
#include <cstddef>
#include <memory>
#include <string>
#include <thread>

namespace Logger {

template <typename T> using RB = Ringbuffer::RingBuffer<T>;

namespace {
constexpr auto store_log_try_duration = std::chrono::milliseconds(50);
} // namespace

//===----------------------------------------------------------------------===//

class ConsoleLoggerImpl : public Logger {
public:
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

ConsoleLoggerImpl::ConsoleLoggerImpl(std::size_t queue_size)
    : buffer(std::make_unique<RB<std::string>>(queue_size)),
      log_thread(&ConsoleLoggerImpl::store_logs, this) {}

ConsoleLoggerImpl::~ConsoleLoggerImpl() {
    stop_log_thread = true;
    if (log_thread.joinable()) {
        log_thread.join();
    }
}

void ConsoleLoggerImpl::store_logs() {
    while (!stop_log_thread) {
        RB<std::string>::Result res = buffer->try_pop(store_log_try_duration);

        if (!res.err()) {
            fmt::print("{}", res.data());
        }
    }
}

void ConsoleLoggerImpl::log(LogLevel loglevel, const std::string& logmsg) {
    const auto time_now = std::chrono::system_clock::now();

    fmt::text_style style;
    switch (loglevel) {
    case LogLevel::Debug:
        style = fmt::text_style();
        break;
    case LogLevel::Warn:
        style = fmt::fg(fmt::color::yellow);
        break;
    case LogLevel::Error:
        style = fmt::fg(fmt::color::red);
        break;
    }

    std::string msg = fmt::format(style, "[{}]: {}\n", time_now, logmsg);
    buffer->try_push(msg);
}

auto ConsoleLogger(std::size_t queue_size) -> std::shared_ptr<Logger> {
    std::shared_ptr<Logger> ptr(
        std::make_shared<ConsoleLoggerImpl>(queue_size));
    return ptr;
}

//===----------------------------------------------------------------------===//

class FileLoggerImpl : public Logger {
public:
    FileLoggerImpl(std::size_t queue_size, const std::string& logfile);
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
    void store_logs();
    std::thread log_thread;
};

FileLoggerImpl::FileLoggerImpl(std::size_t queue_size,
                               const std::string& logfile)
    : buffer(std::make_unique<RB<std::string>>(queue_size)),
      file(fmt::output_file(logfile)),
      log_thread(&FileLoggerImpl::store_logs, this) {}

FileLoggerImpl::~FileLoggerImpl() {
    stop_log_thread = true;
    if (log_thread.joinable()) {
        log_thread.join();
    }
}

void FileLoggerImpl::store_logs() {
    while (!stop_log_thread) {
        RB<std::string>::Result res = buffer->try_pop(store_log_try_duration);

        if (!res.err()) {
            file.print("{}", res.data());
        }
    }
}

void FileLoggerImpl::log(LogLevel loglevel, const std::string& logmsg) {
    const auto time_now = std::chrono::system_clock::now();

    std::string msglabel;
    switch (loglevel) {
    case LogLevel::Debug:
        msglabel = "Debug";
        break;
    case LogLevel::Warn:
        msglabel = "Warn";
        break;
    case LogLevel::Error:
        msglabel = "Error";
        break;
    }

    std::string msg = fmt::format("[{}] {}: {}\n", time_now, msglabel, logmsg);
    buffer->try_push(msg);
}

auto FileLogger(const std::string& logfile,
                                   std::size_t queue_size) -> std::shared_ptr<Logger> {
    std::shared_ptr<Logger> ptr(
        std::make_shared<FileLoggerImpl>(queue_size, logfile));
    return ptr;
}

} // namespace Logger
