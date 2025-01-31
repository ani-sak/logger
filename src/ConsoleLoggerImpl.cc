#include "ConsoleLoggerImpl.hpp"
#include "async_logger/logger.hpp"
#include "fmt/chrono.h"
#include "fmt/color.h"
#include "ringbuffer.hpp"
#include <memory>

namespace Logger {

namespace {
constexpr auto store_log_try_duration = std::chrono::milliseconds(50);
} // namespace

ConsoleLoggerImpl::ConsoleLoggerImpl(std::size_t queue_size,
                                     LogStrategy log_strategy)
    : buffer(std::make_unique<RB<std::string>>(queue_size)),
      log_strategy(log_strategy),
      log_thread(&ConsoleLoggerImpl::store_logs, this) {}

ConsoleLoggerImpl::~ConsoleLoggerImpl() {
    stop_log_thread = true;
    if (log_thread.joinable()) {
        log_thread.join();
    }
}

void ConsoleLoggerImpl::store_logs() {
    while (!stop_log_thread) {
        RB<std::string>::Result res = buffer->try_pop(store_log_try_duration);

        if (!res.err()) {
            fmt::print("{}", res.data());
        }
    }
}

void ConsoleLoggerImpl::log(LogLevel loglevel, const std::string& logmsg) {
    const auto time_now = std::chrono::system_clock::now();

    fmt::text_style style;
    switch (loglevel) {
    case LogLevel::Debug:
        style = fmt::text_style();
        break;
    case LogLevel::Warn:
        style = fmt::fg(fmt::color::yellow);
        break;
    case LogLevel::Error:
        style = fmt::fg(fmt::color::red);
        break;
    }

    std::string msg = fmt::format(style, "[{}]: {}\n", time_now, logmsg);

    switch (log_strategy) {
    case LogStrategy::Blocking:
        buffer->push(msg);
        break;
    case LogStrategy::Immediate:
        buffer->try_push(msg);
        break;
    }
}

} // namespace Logger
