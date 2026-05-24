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

// logbuf is packed as follows:
// [Loginfo, Log, Padding, LogInfo, Log, Padding ...]
// LogInfo contains metadata of the log
// Log is an array of chars of length specified in the previous LogInfo log_size
// Padding to align next LogInfo
struct Buffer {
    char* logbuf = nullptr;
    std::size_t logbuf_size_bytes = 0;
    std::size_t idx = 0;
    bool user_alloc = false;
};

auto verify_alloc(Buffer* buf, std::size_t log_size_bytes) -> bool {
    std::size_t padding =
        (alignof(LogInfo) - (buf->idx % alignof(LogInfo))) % alignof(LogInfo);
    return buf->idx + padding + sizeof(LogInfo) + log_size_bytes <=
           buf->logbuf_size_bytes;
}

auto alloc(Buffer* buf, LogInfo log_info, void const* log) -> void {
    std::size_t padding =
        (alignof(LogInfo) - (buf->idx % alignof(LogInfo))) % alignof(LogInfo);

    char* base = buf->logbuf + buf->idx + padding;

    auto* info = reinterpret_cast<LogInfo*>(base);
    info->log_level = log_info.log_level;
    info->log_size = log_info.log_size;

    void* dest = static_cast<void*>(base + sizeof(LogInfo));
    std::memcpy(dest, log, info->log_size);

    buf->idx += padding + sizeof(LogInfo) + info->log_size;
}

auto create_buffer(std::size_t buffer_size_bytes)
    -> Buffer* {
    auto* buffer = static_cast<Buffer*>(malloc(sizeof(Buffer)));
    char* logbuf = static_cast<char*>(malloc(buffer_size_bytes));
    if (buffer == nullptr || logbuf == nullptr) {
        return nullptr;
    }

    buffer->logbuf = logbuf;
    buffer->logbuf_size_bytes = buffer_size_bytes;
    buffer->idx = 0;
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

    free(buffer->logbuf);
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
    if (!verify_alloc(buffer, logmsg.size())) {
        return false;
    }

    LogInfo info;
    info.log_level = loglevel;
    info.log_size = logmsg.size();
    alloc(buffer, info, logmsg.data());

    return true;
}

auto log(Buffer* buffer, LogLevel loglevel, const char* logmsg_c)
    -> bool {
    if (loglevel > log_level_program) {
        return false;
    }

    std::string logmsg(logmsg_c);

    if (!verify_alloc(buffer, logmsg.size())) {
        return false;
    }

    LogInfo info{};
    info.log_level = loglevel;
    info.log_size = logmsg.size();
    alloc(buffer, info, logmsg.data());

    return true;
}

auto log(Buffer* buffer, LogLevel loglevel,
         std::string_view logmsg) -> bool {
    if (loglevel > log_level_program) {
        return false;
    }
    if (!verify_alloc(buffer, logmsg.size())) {
        return false;
    }

    LogInfo info;
    info.log_level = loglevel;
    info.log_size = logmsg.size();
    alloc(buffer, info, logmsg.data());

    return true;
}

auto flush(Buffer* buf) -> bool {
    int32_t idx = 0;
    while (idx < buf->idx) {
        std::size_t padding =
            (alignof(LogInfo) - (idx % alignof(LogInfo))) % alignof(LogInfo);

        char* base = buf->logbuf + idx + padding;

        auto* info = reinterpret_cast<LogInfo*>(base);
        fmt::text_style style;
        switch (info->log_level) {
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

        char* dest = (base + sizeof(LogInfo));
        std::string_view log((dest), info->log_size);
        fmt::print(fmt::format(style, "{}\n", log));

        idx += padding + sizeof(LogInfo) + info->log_size;
    }

    buf->idx = 0;
    return true;
}

} // namespace Logger
