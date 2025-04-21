#include "file_logger_thread.hpp"
#include "fmt/os.h"

namespace Logger {

FileLoggerThread::FileLoggerThread()
    : log_thread(&FileLoggerThread::store_logs, this) {}

FileLoggerThread::~FileLoggerThread() {
    stop_log_thread = true;

    if (log_thread.joinable()) {
        log_thread.join();
    }
}

auto FileLoggerThread::store_logs() -> void {
    constexpr auto store_log_try_duration = std::chrono::milliseconds(50);

    while (!stop_log_thread) {
        std::lock_guard<std::mutex> lock{logger_vector_mtx};

        for (auto& file_logger_impl : file_loggers_immediate) {
            Ringbuffer::RingBuffer<std::string>::Result res =
                file_logger_impl->buffer->try_pop(store_log_try_duration);

            if (!res.err()) {
                file_logger_impl->file.print("{}", res.data());
            }
        }

        for (auto& file_logger_impl : file_loggers_blocking) {
            Ringbuffer::RingBuffer<std::string>::Result res =
                file_logger_impl->buffer->try_pop(store_log_try_duration);

            if (!res.err()) {
                file_logger_impl->file.print("{}", res.data());
            }
        }
    }
}

} // namespace Logger
