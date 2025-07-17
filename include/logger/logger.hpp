#ifndef LOGGER_LOGGER_HPP
#define LOGGER_LOGGER_HPP

#include <memory>
#include <string>
#include <string_view>

namespace Logger {

constexpr std::size_t default_buffer_size = 64;
constexpr std::size_t default_entry_size = 1024;

enum class LogLevel { Error, Warn, Debug };

auto set_log_level(LogLevel log_level) -> void;

class Buffer; // Forward Declaration

// Use shared_ptr over pImpl to adhere to standard
auto create_buffer(std::size_t buffer_size = default_buffer_size,
                   std::size_t entry_size = default_entry_size)
    -> std::shared_ptr<Buffer>;

auto log(std::shared_ptr<Buffer> buffer, LogLevel loglevel,
         const std::string& logmsg) -> bool;
auto log(std::shared_ptr<Buffer> buffer, LogLevel loglevel, const char* logmsg)
    -> bool;
auto log(std::shared_ptr<Buffer> buffer, LogLevel loglevel,
         std::string_view logmsg) -> bool;

auto flush(std::shared_ptr<Buffer> buffer) -> bool; // Flush to terminal
auto flush(std::shared_ptr<Buffer> buffer, const std::string& logfile) -> bool;

} // namespace Logger

#endif // !LOGGER_LOGGER_HPP
