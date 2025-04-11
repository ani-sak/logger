#include "async_logger/logger.hpp"
#include "ConsoleLoggerImpl.hpp"
#include "FileLoggerImpl.hpp"
#include "ringbuffer.hpp"

#include <atomic>
#include <cstddef>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace Logger {

namespace {
std::mutex console_logger_mtx;
std::mutex file_logger_mtx;
} // namespace

auto ConsoleLogger(std::size_t queue_size, LogStrategy log_strategy)
    -> std::shared_ptr<Logger> {
    std::lock_guard<std::mutex> lock{console_logger_mtx};

    static std::shared_ptr<Logger> ptr;

    if (ptr) {
        return ptr;
    }

    switch (log_strategy) {
    case LogStrategy::Blocking:
        ptr = std::make_shared<ConsoleLoggerImpl<LogStrategy::Blocking>>(
            queue_size);
        break;
    case LogStrategy::Immediate:
        ptr = std::make_shared<ConsoleLoggerImpl<LogStrategy::Immediate>>(
            queue_size);
        break;
    }

    return ptr;
}

auto FileLogger(const std::string& logfile, std::size_t queue_size,
                LogStrategy log_strategy) -> std::shared_ptr<Logger> {
    std::lock_guard<std::mutex> lock{file_logger_mtx};

    static std::unordered_map<std::string, std::shared_ptr<Logger>>
        file_logger_map;
    static FileLoggerThread file_logger_thread;

    auto map_entry = file_logger_map.find(logfile);
    if (map_entry != file_logger_map.end()) {
        return map_entry->second;
    }

    switch (log_strategy) {
    case LogStrategy::Blocking: {
        auto logger = std::make_shared<FileLoggerImpl<LogStrategy::Blocking>>(
            queue_size, logfile);

        file_logger_thread.append(logger);
        file_logger_map[logfile] = logger;

        return logger;
    }
    case LogStrategy::Immediate: {
        auto logger = std::make_shared<FileLoggerImpl<LogStrategy::Immediate>>(
            queue_size, logfile);

        file_logger_thread.append(logger);
        file_logger_map[logfile] = logger;

        return logger;
    }
    }
}

} // namespace Logger
