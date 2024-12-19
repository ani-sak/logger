#include "async_logger/logger.hpp"
#include "fmt/base.h"
#include "fmt/format.h"
#include "fmt/chrono.h"

#include <chrono>
#include <cstddef>
#include <memory>

void test_logger(std::shared_ptr<Logger::Logger> logger, std::size_t log_rounds,
                 std::size_t logs_per_round,
                 Logger::LogLevel log_level = Logger::LogLevel::Warn) {
    for (std::size_t rnd = 0; rnd < log_rounds; rnd++) {
        for (std::size_t logidx = 1; logidx <= logs_per_round; logidx++) {
            logger->log(log_level,
                        std::to_string(rnd * logs_per_round + logidx));
        }
        auto time_now = std::chrono::system_clock::now();
        std::string msg = fmt::format("Done logging entries: {},{} at {}\n",
                                      rnd * logs_per_round,
                                      (rnd + 1) * logs_per_round, time_now);
        fmt::print(msg);
    }
}

int main() {
    auto logger = Logger::ConsoleLogger(6);
    auto cout = Logger::FileLogger("testlog.log", 6);
    test_logger(logger, 3, 10, Logger::LogLevel::Error);
    test_logger(cout, 3, 10);
}
