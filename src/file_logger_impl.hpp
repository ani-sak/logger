#ifndef FILE_LOGGER_IMPL_HPP
#define FILE_LOGGER_IMPL_HPP

#include "async_logger/logger.hpp"
#include "fmt/os.h"
#include "fmt/chrono.h"
#include "ringbuffer.hpp"

#include <cstddef>
#include <string>
#include <string_view>

namespace Logger {

template <LogStrategy L>
struct FileLoggerImpl : public Logger {
    template <typename T>
    using RB = Ringbuffer::RingBuffer<T>;

    FileLoggerImpl(std::size_t queue_size, const std::string& logfile);
    FileLoggerImpl(FileLoggerImpl&&) = delete;
    FileLoggerImpl(const FileLoggerImpl&) = delete;
    auto operator=(FileLoggerImpl&&) -> FileLoggerImpl& = delete;
    auto operator=(const FileLoggerImpl&) -> FileLoggerImpl& = delete;
    ~FileLoggerImpl() override = default;

    void log(LogLevel loglevel, const std::string& logmsg) override;

    std::unique_ptr<RB<std::string>> buffer;
    fmt::ostream file;
    auto get_logmsg_label(LogLevel loglevel) -> std::string_view;
};

template <LogStrategy L>
FileLoggerImpl<L>::FileLoggerImpl(std::size_t queue_size,
                                  const std::string& logfile)
    : buffer(std::make_unique<RB<std::string>>(queue_size)),
      file(fmt::output_file(logfile)) {}

template <LogStrategy L>
auto FileLoggerImpl<L>::get_logmsg_label(LogLevel loglevel)
    -> std::string_view {
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
