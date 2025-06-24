#include "async_logger/logger.hpp"

#include <string>

namespace Test {
auto log_cout_basic() -> void {}
auto log_file_basic() -> void {}

auto log_cout_threaded() -> void{}
auto log_file_threaded() -> void{}
}

auto main(int /*argc*/, char* /*argv*/[]) -> int {
    // AsyncLogger::set_log_level(AsyncLogger::LogLevel::Error);

    auto buf_term = AsyncLogger::create_buffer(100);

    for (std::size_t idx = 0; idx < 500; ++idx) {
        AsyncLogger::log(buf_term, AsyncLogger::LogLevel::Error,
                         std::to_string(idx));
    }

    AsyncLogger::flush(buf_term);

    auto buf_file = AsyncLogger::create_buffer(100);

    for (std::size_t idx = 1000; idx < 1500; ++idx) {
        AsyncLogger::log(buf_file, AsyncLogger::LogLevel::Warn, std::to_string(idx));
    }

    AsyncLogger::flush(buf_file, "test.log");
    return 0;
}
