#include "async_logger/logger.hpp"
#include "ConsoleLoggerImpl.hpp"
#include "FileLoggerImpl.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>

namespace Logger {

auto ConsoleLogger(std::size_t queue_size) -> std::shared_ptr<Logger> {
    static std::shared_ptr<Logger> ptr(
        std::make_shared<ConsoleLoggerImpl>(queue_size));
    return ptr;
}

auto FileLogger(const std::string& logfile,
                std::size_t queue_size) -> std::shared_ptr<Logger> {
    static std::unordered_map<std::string, std::shared_ptr<FileLoggerImpl>>
        file_logger_map;

    auto map_entry = file_logger_map.find(logfile);
    if (map_entry != file_logger_map.end())
    {
        return map_entry->second;
    }

    auto res = file_logger_map.emplace(
        logfile, std::make_shared<FileLoggerImpl>(queue_size, logfile));
    return res.first->second;
}

} // namespace Logger
