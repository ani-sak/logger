#include "log.hpp"
#include "ringbuffer.hpp"

#include <chrono>
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

namespace Logging {

constexpr std::size_t default_queue_size = 250;

Logger::Logger()
    : buffer(std::make_unique<RB<std::string>>(default_queue_size)),
      stop_log_thread(false), log_thread(&Logger::store_logs, this) {}

Logger::Logger(std::size_t queue_size)
    : buffer(std::make_unique<RB<std::string>>(queue_size)),
      stop_log_thread(false), log_thread(&Logger::store_logs, this) {}

Logger::~Logger() {
    // Flush buffer

    stop_log_thread = true;
    if (log_thread.joinable()) {
        log_thread.join();
    }
}

void Logger::store_logs() {
    while (!stop_log_thread) {
        RB<std::string>::Result res =
            buffer->try_pop(std::chrono::milliseconds(50));

        if (!res.err()) {
            std::cout << res.data() << '\n';
        }
    }
}

void Logger::log(LogLevel loglevel, const std::string& logmsg) {
    buffer->try_push(logmsg);
}

} // namespace Logging
