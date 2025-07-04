#ifndef ASYNC_LOGGER_LOGGER_HPP
#define ASYNC_LOGGER_LOGGER_HPP

#include <cstddef>
#include <memory>
#include <string>

namespace AsyncLogger {

constexpr std::size_t default_buffer_size = 64;
constexpr std::size_t default_entry_size = 1024;

enum class LogLevel { Error, Warn, Debug };

auto set_log_level(LogLevel log_level) -> void;

class Buffer; // Forward Declaration
// Abstract buffer for cleaner interface.
// This can be done using shared ptr or pImpl.
// Choose shared ptr approach for code reuse.

auto create_buffer(std::size_t buffer_size = default_buffer_size,
                   std::size_t entry_size = default_entry_size)
    -> std::shared_ptr<Buffer>;

// Goal: avoid unnecessary operations when copying logmsg to ringbuffer
// Option: Universal reference + perfect forwarding
//  logmsg is universal reference.
//  log definition in .hpp, breaks clean interface
//  log definition in .cpp, requires explicit template instantiation
//      difficult with universal references
// Solution: Seperate overloads
//  std::string overload:
//      std::string lvalues require a copy regardless.
//      std::string rvalues are moved in the function, avoiding extra copies.
//  const char * overload to avoid unnecessary creating std::string temporary.
auto log(std::shared_ptr<Buffer> buffer, LogLevel loglevel,
         const std::string& logmsg) -> bool;
auto log(std::shared_ptr<Buffer> buffer, LogLevel loglevel, const char* logmsg)
    -> bool;

auto flush(std::shared_ptr<Buffer> buffer) -> bool;
auto flush(std::shared_ptr<Buffer> buffer, const std::string& logfile) -> bool;
} // namespace AsyncLogger

#endif // !ASYNC_LOGGER_LOGGER_HPP
