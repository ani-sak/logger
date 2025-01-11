#include "FileLoggerImpl.hpp"
#include "fmt/chrono.h"

namespace Logger {

namespace {
constexpr auto store_log_try_duration = std::chrono::milliseconds(50);
} // namespace

FileLoggerImpl::FileLoggerImpl(std::size_t queue_size,
                               const std::string& logfile)
    : buffer(std::make_unique<RB<std::string>>(queue_size)),
      file(fmt::output_file(logfile)),
      log_thread(&FileLoggerImpl::store_logs, this) {}

FileLoggerImpl::~FileLoggerImpl() {
    stop_log_thread = true;
    if (log_thread.joinable()) {
        log_thread.join();
    }
}

void FileLoggerImpl::store_logs() {
    while (!stop_log_thread) {
        RB<std::string>::Result res = buffer->try_pop(store_log_try_duration);

        if (!res.err()) {
            file.print("{}", res.data());
        }
    }
}

void FileLoggerImpl::log(LogLevel loglevel, const std::string& logmsg) {
    const auto time_now = std::chrono::system_clock::now();

    std::string msglabel;
    switch (loglevel) {
    case LogLevel::Debug:
        msglabel = "Debug";
        break;
    case LogLevel::Warn:
        msglabel = "Warn";
        break;
    case LogLevel::Error:
        msglabel = "Error";
        break;
    }

    std::string msg = fmt::format("[{}] {}: {}\n", time_now, msglabel, logmsg);
    buffer->try_push(msg);
}

} // namespace Logger
