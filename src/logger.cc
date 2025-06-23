#include "async_logger/logger.hpp"
#include "fmt/base.h"
#include "fmt/color.h"
#include "fmt/os.h"
#include "ringbuffer.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <utility>

namespace AsyncLogger {

struct LogEntry {
    LogLevel log_level;
    std::string log_msg;

    LogEntry() = default;

    template <typename T>
    LogEntry(LogLevel log_level, T&& log_msg)
        : log_level{log_level}, log_msg{std::forward<T>(log_msg)} {}

    LogEntry(LogEntry &&) = default;
    LogEntry(const LogEntry &) = default;
    auto operator=(LogEntry &&) -> LogEntry & = default;
    auto operator=(const LogEntry &) -> LogEntry & = default;
    ~LogEntry() = default;
};

using RB = Ringbuffer::RingBuffer<LogEntry>;

enum class LogLocation { Term, File };

// Buffer associated with LogLocation so flush API does not have to keep track
// of open file descriptors and synchronize reads/writes from multiple threads.
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
    -> SharedPtrBuffer {
    return std::make_shared<Buffer>(buffer_size, entry_size);
}

auto create_buffer(const std::string& logfile, std::size_t buffer_size,
                   std::size_t entry_size) -> SharedPtrBuffer {
    return std::make_shared<Buffer>(logfile, buffer_size, entry_size);
}

auto log(SharedPtrBuffer buffer, LogLevel loglevel, const std::string& logmsg)
    -> bool {
    return buffer->ringbuffer.try_push(LogEntry{loglevel, logmsg});
}

auto flush(SharedPtrBuffer buffer) -> bool {
    RB::Result res = buffer->ringbuffer.try_pop();

    if (buffer->log_location == LogLocation::Term) {
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
    }

    if (buffer->log_location == LogLocation::File) {
        fmt::ostream ofs{fmt::output_file(buffer->logfile)};

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

            res = buffer->ringbuffer.try_pop();
        }
    }

    return true;
}

} // namespace AsyncLogger
