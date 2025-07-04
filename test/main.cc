#include "async_logger/logger.hpp"

#include <string>

auto main(int /*argc*/, char* /*argv*/[]) -> int {
    auto buf_term = AsyncLogger::create_buffer(100);

    std::string str_lvalue{"string lvalue"};
    AsyncLogger::log(buf_term, AsyncLogger::LogLevel::Error, str_lvalue);

    AsyncLogger::log(buf_term, AsyncLogger::LogLevel::Error,
                     std::string{"string rvalue"});
    AsyncLogger::log(buf_term, AsyncLogger::LogLevel::Error,
                     "const char pointer");

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
