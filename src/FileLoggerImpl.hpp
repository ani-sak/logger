#ifndef FILE_LOGGER_IMPL_HPP
#define FILE_LOGGER_IMPL_HPP

#include "async_logger/logger.hpp"
#include "fmt/os.h"
#include "ringbuffer.hpp"

#include <atomic>
#include <cstddef>
#include <string>
#include <string_view>
#include <thread>

namespace Logger {

template <LogStrategy L>
class FileLoggerImpl : public Logger {
public:
    template <typename T>
    using RB = Ringbuffer::RingBuffer<T>;

    FileLoggerImpl(std::size_t queue_size, const std::string& logfile);
    FileLoggerImpl(FileLoggerImpl&&) = delete;
    FileLoggerImpl(const FileLoggerImpl&) = delete;
    auto operator=(FileLoggerImpl&&) -> FileLoggerImpl& = delete;
    auto operator=(const FileLoggerImpl&) -> FileLoggerImpl& = delete;
    ~FileLoggerImpl() override;

    void log(LogLevel loglevel, const std::string& logmsg) override;

private:
    std::unique_ptr<RB<std::string>> buffer;
    fmt::ostream file;
    std::atomic_bool stop_log_thread{};
    void store_logs();
    std::thread log_thread;
    std::string_view get_logmsg_label(LogLevel loglevel);
};

template <LogStrategy L>
FileLoggerImpl<L>::FileLoggerImpl(std::size_t queue_size,
                                  const std::string& logfile)
    : buffer(std::make_unique<RB<std::string>>(queue_size)),
      file(fmt::output_file(logfile)),
      log_thread(&FileLoggerImpl<L>::store_logs, this) {}

template <LogStrategy L>
FileLoggerImpl<L>::~FileLoggerImpl() {
    stop_log_thread = true;
    if (log_thread.joinable()) {
        log_thread.join();
    }
}

template <LogStrategy L>
void FileLoggerImpl<L>::store_logs() {
    constexpr auto store_log_try_duration = std::chrono::milliseconds(50);

    while (!stop_log_thread) {
        RB<std::string>::Result res = buffer->try_pop(store_log_try_duration);

        if (!res.err()) {
            file.print("{}", res.data());
        }
    }
}

template <LogStrategy L>
std::string_view FileLoggerImpl<L>::get_logmsg_label(LogLevel loglevel) {
    switch (loglevel) {
    case LogLevel::Debug:
        return "Debug";
    case LogLevel::Warn:
        return "Warn";
    case LogLevel::Error:
        return "Error";
    }
}

template <>
inline void
FileLoggerImpl<LogStrategy::Blocking>::log(LogLevel loglevel,
                                           const std::string& logmsg) {
    const auto time_now = std::chrono::system_clock::now();

    std::string_view msglabel = get_logmsg_label(loglevel);
    std::string msg = fmt::format("[{}] {}: {}\n", time_now, msglabel, logmsg);

    buffer->push(msg);
}

template <>
inline void
FileLoggerImpl<LogStrategy::Immediate>::log(LogLevel loglevel,
                                            const std::string& logmsg) {
    const auto time_now = std::chrono::system_clock::now();

    std::string_view msglabel = get_logmsg_label(loglevel);
    std::string msg = fmt::format("[{}] {}: {}\n", time_now, msglabel, logmsg);

    buffer->try_push(msg);
}

} // namespace Logger

#endif // !FILE_LOGGER_IMPL_HPP
