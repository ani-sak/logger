#include "async_logger/logger.hpp"

#include <string>

 auto main (int  /*argc*/, char * /*argv*/[]) -> int {
    auto buf = AsyncLogger::create_buffer(98);

    for (std::size_t idx = 0; idx < 50; ++idx) {
        AsyncLogger::log(buf, AsyncLogger::LogLevel::Error,
                         std::to_string(idx));
    }

    AsyncLogger::flush(buf);

    return 0;
}
