#include "async_logger/logger.hpp"
#include "fmt/base.h"
#include "fmt/format.h"
#include "fmt/chrono.h"

#include <chrono>
#include <cstddef>
#include <memory>
#include <thread>

void logger_test_helper(const std::shared_ptr<Logger::Logger>& logger, std::size_t log_rounds,
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

void test_console_logger() {
    constexpr std::size_t buffer_size = 6;
    constexpr std::size_t logs_per_round = 10;

    auto cout_1 =
        Logger::ConsoleLogger(buffer_size, Logger::LogStrategy::Blocking);
    auto cout_2 = Logger::ConsoleLogger(buffer_size);

    std::thread console_log_thread_1(&logger_test_helper, cout_1, 3,
                                     logs_per_round, Logger::LogLevel::Error);
    std::thread console_log_thread_2(&logger_test_helper, cout_2, 3,
                                     logs_per_round, Logger::LogLevel::Warn);

    console_log_thread_1.join();
    console_log_thread_2.join();
}

void test_file_logger() {
    constexpr std::size_t buffer_size = 6;
    constexpr std::size_t logs_per_round = 50;

    auto filelog_1 = Logger::FileLogger("testlog.log", buffer_size);
    auto filelog_2 = Logger::FileLogger("testlog.log", buffer_size);
    auto filelog_3 = Logger::FileLogger("testlog.log", buffer_size);

    std::thread file_log_thread_1(&logger_test_helper, filelog_1, 3,
                                     logs_per_round, Logger::LogLevel::Error);
    std::thread file_log_thread_2(&logger_test_helper, filelog_2, 3,
                                     logs_per_round, Logger::LogLevel::Warn);
    std::thread file_log_thread_3(&logger_test_helper, filelog_3, 3,
                                     logs_per_round, Logger::LogLevel::Warn);

    file_log_thread_1.join();
    file_log_thread_2.join();
    file_log_thread_3.join();
}

auto main() -> int {
    test_console_logger();
    test_file_logger();
}
