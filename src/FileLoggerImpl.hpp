#ifndef FILE_LOGGER_IMPL_HPP
#define FILE_LOGGER_IMPL_HPP

#include "async_logger/logger.hpp"
#include "fmt/os.h"
#include "ringbuffer.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace Logger {

template <LogStrategy L>
struct FileLoggerImpl : public Logger {
    template <typename T>
    using RB = Ringbuffer::RingBuffer<T>;

    FileLoggerImpl(std::size_t queue_size, const std::string& logfile);
    FileLoggerImpl(FileLoggerImpl&&) = delete;
    FileLoggerImpl(const FileLoggerImpl&) = delete;
    auto operator=(FileLoggerImpl&&) -> FileLoggerImpl& = delete;
    auto operator=(const FileLoggerImpl&) -> FileLoggerImpl& = delete;
    ~FileLoggerImpl() override = default;

    void log(LogLevel loglevel, const std::string& logmsg) override;

    std::unique_ptr<RB<std::string>> buffer;
    fmt::ostream file;
    auto get_logmsg_label(LogLevel loglevel) -> std::string_view;
};

template <LogStrategy L>
FileLoggerImpl<L>::FileLoggerImpl(std::size_t queue_size,
                                  const std::string& logfile)
    : buffer(std::make_unique<RB<std::string>>(queue_size)),
      file(fmt::output_file(logfile)) {}

template <LogStrategy L>
auto FileLoggerImpl<L>::get_logmsg_label(LogLevel loglevel)
    -> std::string_view {
    switch (loglevel) {
    case LogLevel::Debug:
        return "Debug";
    case LogLevel::Warn:
        return "Warn";
    case LogLevel::Error:
        return "Error";
    }
}

template <>
inline void
FileLoggerImpl<LogStrategy::Blocking>::log(LogLevel loglevel,
                                           const std::string& logmsg) {
    const auto time_now = std::chrono::system_clock::now();

    std::string_view msglabel = get_logmsg_label(loglevel);
    std::string msg = fmt::format("[{}] {}: {}\n", time_now, msglabel, logmsg);

    buffer->push(msg);
}

template <>
inline void
FileLoggerImpl<LogStrategy::Immediate>::log(LogLevel loglevel,
                                            const std::string& logmsg) {
    const auto time_now = std::chrono::system_clock::now();

    std::string_view msglabel = get_logmsg_label(loglevel);
    std::string msg = fmt::format("[{}] {}: {}\n", time_now, msglabel, logmsg);

    buffer->try_push(msg);
}

class FileLoggerThread {
public:
    FileLoggerThread();
    FileLoggerThread(FileLoggerThread&&) = delete;
    FileLoggerThread(const FileLoggerThread&) = delete;
    auto operator=(FileLoggerThread&&) -> FileLoggerThread& = delete;
    auto operator=(const FileLoggerThread&) -> FileLoggerThread& = delete;
    ~FileLoggerThread();

    template <LogStrategy L>
    void append(const std::shared_ptr<FileLoggerImpl<L>>& logger);

private:
    std::mutex logger_vector_mtx;
    std::vector<std::shared_ptr<FileLoggerImpl<LogStrategy::Immediate>>>
        file_loggers_immediate;
    std::vector<std::shared_ptr<FileLoggerImpl<LogStrategy::Blocking>>>
        file_loggers_blocking;

    void store_logs();
    std::atomic<bool> stop_log_thread{};
    std::thread log_thread;
};

FileLoggerThread::FileLoggerThread()
    : log_thread(&FileLoggerThread::store_logs, this) {}

FileLoggerThread::~FileLoggerThread() {
    stop_log_thread = true;

    if (log_thread.joinable()) {
        log_thread.join();
    }
}

template <>
void FileLoggerThread::append(
    const std::shared_ptr<FileLoggerImpl<LogStrategy::Immediate>>& logger) {
    std::lock_guard<std::mutex> lock{logger_vector_mtx};
    file_loggers_immediate.push_back(logger);
}

template <>
void FileLoggerThread::append(
    const std::shared_ptr<FileLoggerImpl<LogStrategy::Blocking>>& logger) {
    std::lock_guard<std::mutex> lock{logger_vector_mtx};
    file_loggers_blocking.push_back(logger);
}

auto FileLoggerThread::store_logs() -> void {
    constexpr auto store_log_try_duration = std::chrono::milliseconds(50);

    while (!stop_log_thread) {
        std::lock_guard<std::mutex> lock{logger_vector_mtx};

        for (auto& file_logger_impl : file_loggers_immediate) {
            Ringbuffer::RingBuffer<std::string>::Result res =
                file_logger_impl->buffer->try_pop(store_log_try_duration);

            if (!res.err()) {
                file_logger_impl->file.print("{}", res.data());
            }
        }

        for (auto& file_logger_impl : file_loggers_blocking) {
            Ringbuffer::RingBuffer<std::string>::Result res =
                file_logger_impl->buffer->try_pop(store_log_try_duration);

            if (!res.err()) {
                file_logger_impl->file.print("{}", res.data());
            }
        }
    }
}

} // namespace Logger

#endif // !FILE_LOGGER_IMPL_HPP
