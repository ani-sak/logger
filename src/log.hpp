#pragma once

//===----------------------------------------------------------------------===//
//
// Non singleton async logger
// - Allow user to decide no of instances
// - Permits easier testing
//===----------------------------------------------------------------------===//

#include <cstddef>
#include <string>
#include <thread>

#include "ringbuffer_fwd.hpp"

namespace Logging {

template<typename T>
using RB = Ringbuffer::RingBuffer<T>;

class Logger {
public:
    Logger(std::size_t queue_size);
    Logger();
    Logger(Logger&&) = delete;
    Logger(const Logger&) = delete;
    Logger& operator=(Logger&&) = delete;
    Logger& operator=(const Logger&) = delete;
    ~Logger();

    enum class LogLevel { Debug, Warn, Error };
    void log(LogLevel loglevel, const std::string& logmsg);

private:
    std::unique_ptr<RB<std::string>> buffer;
    bool stop_log_thread;
    void store_logs();
    std::thread log_thread;
};

} // namespace Logging
