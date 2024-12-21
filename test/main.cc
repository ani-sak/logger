#include "async_logger/logger.hpp"
#include "fmt/base.h"
#include "fmt/format.h"
#include "fmt/chrono.h"

#include <chrono>
#include <cstddef>
#include <memory>
#include <thread>

void test_logger(const std::shared_ptr<Logger::Logger>& logger, std::size_t log_rounds,
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

auto main() -> int {
    // constexpr std::size_t buffer_sizes = 6;
    // auto logger = Logger::ConsoleLogger(buffer_sizes);
    // auto cout = Logger::FileLogger("testlog.log", buffer_sizes);

    auto cout_1 = Logger::ConsoleLogger();
    auto cout_2 = Logger::ConsoleLogger();
    auto file_logger_1 = Logger::FileLogger("test/testlog.log");
    auto file_logger_2 = Logger::FileLogger("test/testlog.log");

    constexpr std::size_t logs_per_round = 10;

    std::thread console_log_thread_1(&test_logger, cout_1, 3, logs_per_round,
                                   Logger::LogLevel::Error);
    std::thread console_log_thread_2(&test_logger, cout_2, 3, logs_per_round,
                                     Logger::LogLevel::Warn);
    std::thread file_log_thread_1(&test_logger, file_logger_1, 3,
                                  logs_per_round, Logger::LogLevel::Debug);
    std::thread file_log_thread_2(&test_logger, file_logger_2, 3,
                                  logs_per_round, Logger::LogLevel::Debug);

    console_log_thread_1.join();
    console_log_thread_2.join();
    file_log_thread_1.join();
    file_log_thread_2.join();
}
