#ifndef CONSOLE_LOGGER_IMPL_HPP
#define CONSOLE_LOGGER_IMPL_HPP

#include "async_logger/logger.hpp"
#include "fmt/chrono.h"
#include "fmt/color.h"
#include "ringbuffer.hpp"

#include <atomic>
#include <memory>
#include <string>
#include <thread>

namespace Logger {

template <LogStrategy L>
class ConsoleLoggerImpl : public Logger {
public:
    template <typename T>
    using RB = Ringbuffer::RingBuffer<T>;

    ConsoleLoggerImpl(std::size_t queue_size);
    ConsoleLoggerImpl(ConsoleLoggerImpl&&) = delete;
    ConsoleLoggerImpl(const ConsoleLoggerImpl&) = delete;
    auto operator=(ConsoleLoggerImpl&&) -> ConsoleLoggerImpl& = delete;
    auto operator=(const ConsoleLoggerImpl&) -> ConsoleLoggerImpl& = delete;
    ~ConsoleLoggerImpl() override;

    void log(LogLevel loglevel, const std::string& logmsg) override;

private:
    std::unique_ptr<RB<std::string>> buffer;
    std::atomic_bool stop_log_thread{};
    void store_logs();
    std::thread log_thread;
    constexpr auto get_text_style(LogLevel loglevel) -> fmt::text_style;
};

template <LogStrategy L>
ConsoleLoggerImpl<L>::ConsoleLoggerImpl(std::size_t queue_size)
    : buffer(std::make_unique<RB<std::string>>(queue_size)),
      log_thread(&ConsoleLoggerImpl<L>::store_logs, this) {}

template <LogStrategy L>
ConsoleLoggerImpl<L>::~ConsoleLoggerImpl<L>() {
    stop_log_thread = true;
    if (log_thread.joinable()) {
        log_thread.join();
    }
}

template <LogStrategy L>
void ConsoleLoggerImpl<L>::store_logs() {
    constexpr auto store_log_try_duration = std::chrono::milliseconds(50);

    while (!stop_log_thread) {
        RB<std::string>::Result res = buffer->try_pop(store_log_try_duration);

        if (!res.err()) {
            fmt::print("{}", res.data());
        }
    }
}

template <LogStrategy L>
constexpr auto ConsoleLoggerImpl<L>::get_text_style(LogLevel loglevel)
    -> fmt::text_style {
    switch (loglevel) {
    case LogLevel::Debug:
        return {};
    case LogLevel::Warn:
        return fmt::fg(fmt::color::yellow);
    case LogLevel::Error:
        return fmt::fg(fmt::color::red);
    }

    return {};
}

template <>
inline void
ConsoleLoggerImpl<LogStrategy::Blocking>::log(LogLevel loglevel,
                                              const std::string& logmsg) {
    const auto time_now = std::chrono::system_clock::now();

    fmt::text_style style = get_text_style(loglevel);
    std::string msg = fmt::format(style, "[{}]: {}\n", time_now, logmsg);

    buffer->push(msg);
}

template <>
inline void
ConsoleLoggerImpl<LogStrategy::Immediate>::log(LogLevel loglevel,
                                               const std::string& logmsg) {
    const auto time_now = std::chrono::system_clock::now();

    fmt::text_style style = get_text_style(loglevel);

    std::string msg = fmt::format(style, "[{}]: {}\n", time_now, logmsg);
    buffer->try_push(msg);
}

} // namespace Logger

#endif // !CONSOLE_LOGGER_IMPL_HPP
