#ifndef LOGGER_LOGGER_HPP
#define LOGGER_LOGGER_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace Logger {

constexpr std::size_t default_buffer_size_bytes = 1<<14;

enum class LogLevel : uint8_t { Error = 0, Warn = 1, Debug = 2 };

auto set_log_level(LogLevel log_level) -> void;

class Buffer; // Forward Declaration

auto create_buffer(std::size_t buffer_size_bytes = default_buffer_size_bytes)
    -> Buffer*;
auto create_buffer(void* buffer_user, std::size_t buffer_user_size_bytes)
    -> Buffer*;

auto free_buffer(Buffer* buffer) -> void;

auto log(Buffer* buffer, LogLevel loglevel, const std::string& logmsg) -> bool;
auto log(Buffer* buffer, LogLevel loglevel, const char* logmsg) -> bool;
auto log(Buffer* buffer, LogLevel loglevel, std::string_view logmsg) -> bool;

auto flush(Buffer* buffer) -> bool; // Flush to terminal
auto flush(Buffer* buffer, const std::string& logfile) -> bool;

} // namespace Logger

#endif // !LOGGER_LOGGER_HPP
