#include "async_logger/logger.hpp"
#include "console_logger_impl.hpp"
#include "file_logger_impl.hpp"
#include "file_logger_thread.hpp"
#include "fmt/base.h"
#include "fmt/color.h"
#include "fmt/os.h"
#include "ringbuffer.hpp"

#include <cstddef>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>

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

namespace AsyncLogger {

struct LogEntry {
    LogLevel log_level;
    std::string log_msg;

    LogEntry() = default;
    LogEntry(LogEntry &&) = default;
    LogEntry(const LogEntry &) = default;
    auto operator=(LogEntry &&) -> LogEntry & = default;
    auto operator=(const LogEntry &) -> LogEntry & = default;
    ~LogEntry() = default;
};

using RB = Ringbuffer::RingBuffer<LogEntry>;

enum class LogLocation { Term, File };

struct Buffer {
public:
    Buffer(std::size_t buffer_size, std::size_t entry_size)
        : ringbuffer{buffer_size}, log_location{LogLocation::Term} {
        ringbuffer.map([entry_size](LogEntry log_entry) {
            log_entry.log_msg.reserve(entry_size);
        });
    }

    Buffer(std::string logfile, std::size_t buffer_size, std::size_t entry_size)
        : ringbuffer{buffer_size}, log_location{LogLocation::File},
          logfile{std::move(logfile)} {
        ringbuffer.map([entry_size](LogEntry log_entry) {
            log_entry.log_msg.reserve(entry_size);
        });
    }

    RB ringbuffer;
    LogLocation log_location;
    std::string logfile;
};

auto create_buffer(std::size_t buffer_size, std::size_t entry_size)
    -> std::shared_ptr<Buffer> {
    return std::make_shared<Buffer>(buffer_size, entry_size);
}

auto create_buffer(const std::string& logfile, std::size_t buffer_size,
                   std::size_t entry_size) -> std::shared_ptr<Buffer> {
    return std::make_shared<Buffer>(logfile, buffer_size, entry_size);
}

auto log(Buffer& buffer, LogLevel loglevel, const std::string& logmsg) -> bool {
    return buffer.ringbuffer.try_push(LogEntry{loglevel, logmsg});
}

auto flush(Buffer& buffer) -> bool {
    RB::Result res = buffer.ringbuffer.try_pop();

    if (buffer.log_location == LogLocation::Term) {
        while (!res.err()) {
            LogEntry log_entry{res.data()};

            fmt::text_style style;
            switch (log_entry.log_level) {
            case LogLevel::Debug:
                style = {};
                break;
            case LogLevel::Warn:
                style = fmt::fg(fmt::color::yellow);
                break;
            case LogLevel::Error:
                style = fmt::fg(fmt::color::red);
                break;
            }

            fmt::print(fmt::format(style, "{} \n", log_entry.log_msg));

            res = buffer.ringbuffer.try_pop();
        }
    }

    if (buffer.log_location == LogLocation::File) {
        fmt::ostream ofs{fmt::output_file(buffer.logfile)};

        while (!res.err()) {
            LogEntry log_entry{res.data()};

            std::string log_prefix;
            switch (log_entry.log_level) {
            case LogLevel::Debug:
                log_prefix =  "Debug";
            case LogLevel::Warn:
                log_prefix = "Warn";
            case LogLevel::Error:
                log_prefix = "Error";
            }

            ofs.print("{}: {} \n", log_prefix, log_entry.log_msg);

            res = buffer.ringbuffer.try_pop();
        }
    }

    return true;
}

} // namespace AsyncLogger
