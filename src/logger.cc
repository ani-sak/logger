#include "async_logger/logger.hpp"
#include "fmt/base.h"
#include "fmt/color.h"
#include "fmt/os.h"
#include "ringbuffer.hpp"

#include <cstddef>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

namespace AsyncLogger {
// The goal is to minimize unnecessary operations when creating buffer entry
// from log function.
// Need to prepend loglevel info.
//
// Approach 1: Ringbuffer of std::string type
//  Here the loglevel info is stored into a string.
//  Need to create string temporary, convert loglevel to string
//
// Approach 2: Ringbuffer of LogEntry type (see analysis below)
//
// Both approaches need creation of a temporary.
//
// Approach 2
// Log overload taking std::string logmsg will bind passed param (lvalue or
// std::string rvalue) to fxn param (std::string const-ref lvalue).
// LogEntry rvalue is created.
//  LogEntry ctor perfect forwards function param logmsg.
//  Note logmsg is lvalue so it is always copied to LogEntry rvalue. (BAD)
//  We want string rvalues to be moved (TODO)
// LogEntry rvalue passed to try_push.
//  try_push forwards LogEntry to push_impl
//  push_impl uses move-assignment to place LogEntry rvalue in buffer
//      move-assignment does element wise move-assignment
//      i.e. string is moved into buffer
// LogEntry rvalue destroyed (BAD)
//
// Log overload taking const char* logmsg only called when const char* passed
// LogEntry rvalue is created.
//  LogEntry ctor perfect forwards function param logmsg.
//  Ctor is templated on const char *
//  const char * logmsg forwarded as rvalue
//  member variable created from const char * constructor
// LogEntry rvalue passed to try_push.
//  try_push forwards LogEntry to push_impl
//  push_impl uses move-assignment to place LogEntry rvalue in buffer
//      move-assignment does element wise move-assignment
//      i.e. string is moved into buffer
// LogEntry rvalue destroyed (BAD)
//
// In both cases an extra temporary (LogEntry) is created and destroyed
//
// Potential Solution: Facilitate copying data directly into reserved string mem
//  Add API to ringbuffer that takes variable no of template params
//  If possible, construct buffer entry using these params
//  This will allow for creating ringbuffer entry from const char* directly
//  To do this in log string overload, use log_msg.c_str()
//
// Both cases the string is moved into buffer
//  Move simply involves swapping internal pointers
//  Ringbuffer entry holds std::string that holds internal pointer to arbitrary
//  memory location
//  We are not using buffer preallocated memory
// This will cause Flush API to be slower (FINE)
//
// TODO:
//  NO SSO: small string optimization used to avoid memory alloc. buffer is
//  pre-allocated so sso useless
//  Copy to buffer: Final operation of log is a copy to the pre-alloc buffer

// store logmsg as std::string for short-string-optimization (SSO)
//
// No SSO, Log Entry has char* with capacity
// Alloc full char buffer in Buffer class
// Each log entry is a pointer to section in buffer i.e.
//  Buffer has ringbuffer of pointers pointing to full char buffer
//  Note Buffer knows full size, per entry size so can handle that
//  log() is simply copying to current ringbuffer head correctly
//      warn if log msg longer than size
//

struct LogEntry {
    LogLevel log_level;
    std::string log_msg;

    LogEntry() = default;

    template <typename T>
    LogEntry(LogLevel log_level, T&& log_msg)
        : log_level{log_level}, log_msg{std::forward<T>(log_msg)} {}

    // Implicitly declared+defined move ctor, move-assignment operator
    // Perform member-wise moves, move-assignment on rvalues
    LogEntry(LogEntry&&) = default;
    auto operator=(LogEntry&&) -> LogEntry& = default;

    LogEntry(const LogEntry&) = default;
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

namespace {
LogLevel log_level_program = LogLevel::Debug;
}

auto set_log_level(LogLevel log_level) -> void {
    log_level_program = log_level;
}

auto log(std::shared_ptr<Buffer> buffer, LogLevel loglevel,
         const std::string& logmsg) -> bool {
    if (log_level_program < loglevel) {
        return false;
    }

    // Pass LogEntry rvalue, which is "perfect forwarded" in try_push call
    return buffer->ringbuffer.try_push(LogEntry{loglevel, logmsg});
}

auto log(std::shared_ptr<Buffer> buffer, LogLevel loglevel, const char* logmsg)
    -> bool {
    if (log_level_program < loglevel) {
        return false;
    }

    // Pass LogEntry rvalue, which is "perfect forwarded" in try_push call
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

        fmt::print(fmt::format(style, "{}\n", log_entry.log_msg));

        res = buffer->ringbuffer.try_pop();
    }

    return true;
}

namespace {
std::mutex map_mutex{};

auto get_logfile_mutex(const std::string& logfile) -> std::mutex& {
    std::lock_guard<std::mutex> lock{map_mutex};
    static std::map<std::string, std::mutex> logfile_mutex_map{};
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
