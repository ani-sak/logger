#include "logger/logger.hpp"

#include "fmt/base.h"
#include "fmt/color.h"
#include "fmt/os.h"

#include <cstddef> // std::size_t
#include <cstdlib>
#include <cstring> // std::memcpy

// Support log API taking string and string_view
#include <string>
#include <string_view>


// Use preallocated ringbuffer, so no benefit of using small string
// optimization. Small string optimization reduces no of (expensive)
// allocations.
//
// Log is not threadsafe. Thread safety is responsibility of library user.
// This avoids overhead of mutex locking in single threaded situations, and
// provides more control to library user.

namespace Logger {

struct LogInfo {
    std::size_t log_size;
    LogLevel log_level;
};

struct Buffer {
    char* logbuf = nullptr;
    std::size_t logbuf_size_bytes = 0;
    std::size_t idx = 0;
    bool user_alloc = false;
};

auto verify_alloc(Buffer* buf, std::size_t log_size_bytes) -> bool {
    return buf->logbuf_size_bytes - buf->idx >= log_size_bytes;
}

const char* prefix_warn = "Warn: ";
const char* prefix_error = "Error: ";
std::size_t prefix_size_max = 7;

auto alloc_prefix(Buffer* buf, Logger::LogLevel log_level) -> void {
    void* dest = static_cast<void*>(buf->logbuf + buf->idx);

    switch (log_level) {
        case Logger::LogLevel::Warn:
            std::memcpy(dest, prefix_warn, 6);
            buf->idx += 6;
            break;
        case Logger::LogLevel::Error:
            std::memcpy(dest, prefix_error, 7);
            buf->idx += 7;
            break;
        default:
            break;
    }
}

auto alloc(Buffer* buf, std::size_t log_size, void const* log) -> void {
    void* base_log = static_cast<void*>(buf->logbuf + buf->idx);
    std::memcpy(base_log, log, log_size);
    buf->idx += log_size;

    buf->logbuf[buf->idx] = '\n';
    buf->idx++;
}

//------------------------------------------------------------------------------

auto create_buffer(std::size_t buffer_size_bytes)
    -> Buffer* {
    if (buffer_size_bytes <= sizeof(Buffer)) {
        return nullptr;
    }

    void* mem = malloc(buffer_size_bytes);
    auto* buffer = create_buffer(mem, buffer_size_bytes);
    buffer->user_alloc = false;
    return buffer;
}

auto create_buffer(void* buffer_user, std::size_t buffer_user_size_bytes)
    -> Buffer* {
    if (buffer_user == nullptr || buffer_user_size_bytes == 0) {
        return nullptr;
    }

    // Assume that buffer_user is aligned!
    auto* buffer = static_cast<Buffer*>(buffer_user);

    char* base = static_cast<char*>(buffer_user);
    buffer->logbuf = base + sizeof(Buffer);
    buffer->logbuf_size_bytes = buffer_user_size_bytes - sizeof(Buffer);

    buffer->idx = 0;
    buffer->user_alloc = true;

    return buffer;
}

auto free_buffer(Buffer* buffer) -> void {
    if (buffer -> user_alloc) {
        return;
    }

    free(buffer);
}

namespace {
LogLevel log_level_program = LogLevel::Debug;
}

auto set_log_level(LogLevel log_level) -> void {
    log_level_program = log_level;
}

auto log(Buffer* buffer, LogLevel loglevel,
         const std::string& logmsg) -> bool {
    if (loglevel > log_level_program) {
        return false;
    }

    std::size_t encoded_logsize =
        prefix_size_max + logmsg.size() + 1; // Add 1 for postfix \n
    if (!verify_alloc(buffer, encoded_logsize)) {
        return false;
    }

    alloc_prefix(buffer, loglevel);
    alloc(buffer, logmsg.size(), logmsg.data());

    return true;
}

auto log(Buffer* buffer, LogLevel loglevel, const char* logmsg_c)
    -> bool {
    if (loglevel > log_level_program) {
        return false;
    }

    std::string logmsg(logmsg_c);

    std::size_t encoded_logsize =
        prefix_size_max + logmsg.size() + 1; // Add 1 for postfix \n
    if (!verify_alloc(buffer, encoded_logsize)) {
        return false;
    }

    alloc_prefix(buffer, loglevel);
    alloc(buffer, logmsg.size(), logmsg.data());

    return true;
}

auto log(Buffer* buffer, LogLevel loglevel,
         std::string_view logmsg) -> bool {
    if (loglevel > log_level_program) {
        return false;
    }

    std::size_t encoded_logsize =
        prefix_size_max + logmsg.size() + 1; // Add 1 for postfix \n
    if (!verify_alloc(buffer, encoded_logsize)) {
        return false;
    }

    alloc_prefix(buffer, loglevel);
    alloc(buffer, logmsg.size(), logmsg.data());

    return true;
}

auto flush(Buffer* buf) -> bool {
    std::string_view log(buf->logbuf, buf->idx);
    fmt::print("{}", log);

    buf->idx = 0;
    return true;
}

auto flush(Buffer* buf, const std::string& logfile) -> bool {
    auto outfile = fmt::output_file(logfile);

    std::string_view log(buf->logbuf, buf->idx);
    outfile.print("{}", log);

    buf->idx = 0;
    return true;
}

} // namespace Logger
