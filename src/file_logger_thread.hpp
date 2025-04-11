#ifndef FILE_LOGGER_THREAD_HPP
#define FILE_LOGGER_THREAD_HPP

#include "file_logger_impl.hpp"
#include "async_logger/logger.hpp"

#include <memory>
#include <thread>
#include <vector>

namespace Logger {

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

template <>
inline void FileLoggerThread::append(
    const std::shared_ptr<FileLoggerImpl<LogStrategy::Immediate>>& logger) {
    std::lock_guard<std::mutex> lock{logger_vector_mtx};
    file_loggers_immediate.push_back(logger);
}

template <>
inline void FileLoggerThread::append(
    const std::shared_ptr<FileLoggerImpl<LogStrategy::Blocking>>& logger) {
    std::lock_guard<std::mutex> lock{logger_vector_mtx};
    file_loggers_blocking.push_back(logger);
}

} // namespace Logger

#endif // !FILE_LOGGER_THREAD_HPP
