#include "async_logger/logger.hpp"
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

namespace AsyncLogger {

struct LogEntry {
    LogLevel log_level;
    std::string log_msg;

    LogEntry() = default;

    template <typename T>
    LogEntry(LogLevel log_level, T&& log_msg)
        : log_level{log_level}, log_msg{std::forward<T>(log_msg)} {}

    LogEntry(LogEntry&&) = default;
    LogEntry(const LogEntry&) = default;
    auto operator=(LogEntry&&) -> LogEntry& = default;
    auto operator=(const LogEntry&) -> LogEntry& = default;
    ~LogEntry() = default;
};

using RB = Ringbuffer::RingBuffer<LogEntry>;

struct Buffer {
public:
    Buffer(std::size_t buffer_size, std::size_t entry_size)
        : ringbuffer{buffer_size} {
        ringbuffer.map([entry_size](LogEntry log_entry) {
            log_entry.log_msg.reserve(entry_size);
        });
    }

    RB ringbuffer;
};

auto create_buffer(std::size_t buffer_size, std::size_t entry_size)
    -> std::shared_ptr<Buffer> {
    return std::make_shared<Buffer>(buffer_size, entry_size);
}

auto log(std::shared_ptr<Buffer> buffer, LogLevel loglevel,
         const std::string& logmsg) -> bool {
    return buffer->ringbuffer.try_push(LogEntry{loglevel, logmsg});
}

namespace {
std::mutex stdout_mtx;
}

auto flush(std::shared_ptr<Buffer> buffer) -> bool {
    std::lock_guard<std::mutex> lock{stdout_mtx};

    RB::Result res = buffer->ringbuffer.try_pop();

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

        res = buffer->ringbuffer.try_pop();
    }

    return true;
}

namespace {
auto get_logfile_mutex(const std::string& logfile) -> std::mutex& {
    static std::unordered_map<std::string, std::mutex> logfile_mutex_map{};
    return logfile_mutex_map[logfile];
}
} // namespace

auto flush(std::shared_ptr<Buffer> buffer, const std::string& logfile) -> bool {
    std::lock_guard<std::mutex> lock{get_logfile_mutex(logfile)};

    fmt::ostream ofs{fmt::output_file(logfile)};

    RB::Result res = buffer->ringbuffer.try_pop();

    while (!res.err()) {
        LogEntry log_entry{res.data()};

        std::string log_prefix;
        switch (log_entry.log_level) {
        case LogLevel::Debug:
            log_prefix = "Debug";
            break;
        case LogLevel::Warn:
            log_prefix = "Warn";
            break;
        case LogLevel::Error:
            log_prefix = "Error";
            break;
        }

        ofs.print("{}: {} \n", log_prefix, log_entry.log_msg);

        res = buffer->ringbuffer.try_pop();
    }

    return true;
}

} // namespace AsyncLogger
