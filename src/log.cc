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
    : buffer(std::make_unique<RingBuffer<std::string>>(default_queue_size)),
      stop_log_thread(false), log_thread(&Logger::store_logs, this) {}

Logger::Logger(std::size_t queue_size)
    : buffer(std::make_unique<RingBuffer<std::string>>(queue_size)),
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
        // RingBuffer<std::string>::Result res = buffer->try_pop();
        // // Perform formatting before storing
        //
        // if (!res.err()) {
        //     std::cout << res.data() << '\n';
        // }
        //
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));

        std::string res = buffer->pop();
        std::cout << res << '\n';
    }
}

void Logger::log(LogLevel loglevel, const std::string &logmsg) {
    // Prepend time
    buffer->try_push(logmsg);
}

} // namespace Logging
