#include "logger/logger.hpp"

#include "fmt/base.h"
#include "fmt/color.h"
#include "fmt/os.h"

#include <cstddef>
#include <map>
#include <mutex>
#include <string_view>
#include <utility>

// Memory is preallocated, do not benefit from small string optimization (which
// reduces expensive allocations)
//
// Favored char* over std::string for easier control over preallocating memory
// and writing to preallocated memory
//
// Log is not currently threadsafe. This is to avoid overhead of mutex locking.
// It is up to clients creating multi-threaded programs to tradeoff/optimize
// overheads of mutex locking and call the library in a theadsafe manner.
namespace AsyncLogger {

// Struct of Arrays (SOA) vs Array of Structs (AOS)
// SOA showed better performance with brief profiling and was chosen
struct Buffer {
public:
    Buffer(std::size_t element_num, std::size_t element_size)
        : item_count{element_num}, item_size{element_size},
          buffer_size_bytes{element_num * element_size},
          buffer_logmsg{new char[buffer_size_bytes]},
          buffer_loglvl{new LogLevel[element_num]},
          buffer_logmsg_size{new std::size_t[element_num]} {}

    const std::size_t item_count;
    const std::size_t item_size;

    std::size_t item_idx_tail = 0;
    std::size_t item_idx_head = 0;
    std::size_t item_idx_valid_count = 0;

    const std::size_t buffer_size_bytes;
    char* buffer_logmsg;
    LogLevel* buffer_loglvl;
    std::size_t* buffer_logmsg_size;

    template <typename T>
    auto push(LogLevel log_level, T&& msg) -> bool {
        if (item_idx_valid_count >= item_count) {
            return false;
        }

        buffer_loglvl[item_idx_tail] = log_level;

        // Unnecessary when T is a std::string lvalue/rvalue as
        // std::string has copy() method
        std::string_view tmp{std::forward<T>(msg)};

        buffer_logmsg_size[item_idx_tail] =
            tmp.copy(buffer_logmsg + (item_idx_tail * item_size), item_size);

        item_idx_tail = (item_idx_tail + 1) % item_count;
        item_idx_valid_count++;

        return true;
    }

    struct LogElement {
        bool valid = false;
        LogLevel lvl;
        std::string_view msg;
    };

    // pop() only returns one log element at a time and therefore needs to be
    // called repeatedly to flush buffer
    //
    // This can be optimized, however flush is not a performance sensitive
    // operation
    auto pop() -> LogElement {
        if (item_idx_valid_count <= 0) {
            return {};
        }

        std::size_t old_head = item_idx_head;
        item_idx_head = (item_idx_head + 1) % item_count;
        item_idx_valid_count--;
        return {true,
                buffer_loglvl[old_head],
                {buffer_logmsg + (old_head * item_size),
                 buffer_logmsg_size[old_head]}};
    }
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

    return buffer->push(loglevel, logmsg);
}

auto log(std::shared_ptr<Buffer> buffer, LogLevel loglevel, const char* logmsg)
    -> bool {
    if (log_level_program < loglevel) {
        return false;
    }

    return buffer->push(loglevel, logmsg);
}

auto log(std::shared_ptr<Buffer> buffer, LogLevel loglevel,
         std::string_view logmsg) -> bool {
    if (log_level_program < loglevel) {
        return false;
    }

    return buffer->push(loglevel, logmsg);
}

namespace {
std::mutex stdout_mtx;
}

auto flush(std::shared_ptr<Buffer> buffer) -> bool {
    std::lock_guard<std::mutex> lock{stdout_mtx};

    auto res = buffer->pop();

    while (res.valid) {
        fmt::text_style style;
        switch (res.lvl) {
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

        fmt::print(fmt::format(style, "{}\n", res.msg));

        res = buffer->pop();
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

    auto res = buffer->pop();

    while (res.valid) {
        std::string_view log_prefix;
        switch (res.lvl) {
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

        ofs.print("{}: {} \n", log_prefix, res.msg);

        res = buffer->pop();
    }

    return true;
}
} // namespace AsyncLogger
