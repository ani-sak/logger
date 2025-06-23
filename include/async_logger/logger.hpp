#ifndef ASYNC_LOGGER_LOGGER_HPP
#define ASYNC_LOGGER_LOGGER_HPP

#include <cstddef>
#include <memory>
#include <string>

namespace AsyncLogger {

constexpr std::size_t default_buffer_size = 64;
constexpr std::size_t default_entry_size = 1024;

enum class LogLevel { Debug, Warn, Error };

class Buffer; // Forward Declaration
using SharedPtrBuffer = std::shared_ptr<Buffer>;

auto create_buffer(std::size_t buffer_size = default_buffer_size,
                   std::size_t entry_size = default_entry_size)
    -> SharedPtrBuffer;

auto create_buffer(const std::string& logfile,
                   std::size_t buffer_size = default_buffer_size,
                   std::size_t entry_size = default_entry_size)
    -> SharedPtrBuffer;

auto log(SharedPtrBuffer buffer, LogLevel loglevel, const std::string& logmsg)
    -> bool;

auto flush(SharedPtrBuffer buffer) -> bool;
} // namespace AsyncLogger

#endif // !ASYNC_LOGGER_LOGGER_HPP
