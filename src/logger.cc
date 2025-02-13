#include "async_logger/logger.hpp"
#include "ConsoleLoggerImpl.hpp"
#include "FileLoggerImpl.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>

namespace Logger {

auto ConsoleLogger(std::size_t queue_size, LogStrategy log_strategy)
    -> std::shared_ptr<Logger> {
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
    static std::unordered_map<std::string, std::shared_ptr<Logger>>
        file_logger_map;

    auto map_entry = file_logger_map.find(logfile);
    if (map_entry != file_logger_map.end()) {
        return map_entry->second;
    }

    switch (log_strategy) {
    case LogStrategy::Blocking: {
        auto res = file_logger_map.emplace(
            logfile, std::make_shared<FileLoggerImpl<LogStrategy::Blocking>>(
                         queue_size, logfile));
        return res.first->second;
    }
    case LogStrategy::Immediate: {
        auto res = file_logger_map.emplace(
            logfile, std::make_shared<FileLoggerImpl<LogStrategy::Immediate>>(
                         queue_size, logfile));
        return res.first->second;
    }
    }
}

} // namespace Logger
