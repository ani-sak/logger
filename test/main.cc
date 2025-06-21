#include "async_logger/logger.hpp"
#include "fmt/base.h"
#include "fmt/chrono.h"
#include "fmt/format.h"

#include <array>
#include <chrono>
#include <cstddef>
#include <iterator>
#include <memory>
#include <string>
#include <thread>

namespace Test {

void logger_test_helper(const std::shared_ptr<Logger::Logger>& logger,
                        std::size_t log_rounds, std::size_t logs_per_round,
                        Logger::LogLevel log_level = Logger::LogLevel::Warn) {
    const auto start_time = std::chrono::high_resolution_clock::now();

    for (std::size_t rnd = 0; rnd < log_rounds; rnd++) {
        for (std::size_t logidx = 1; logidx <= logs_per_round; logidx++) {
            logger->log(log_level,
                        std::to_string((rnd * logs_per_round) + logidx));
        }
        auto time_now = std::chrono::system_clock::now();
    }

    const auto end_time = std::chrono::high_resolution_clock::now();

    fmt::print("Logged {} entries in {} \n", log_rounds * logs_per_round,
               (end_time - start_time));
}

namespace {
constexpr std::size_t buffer_size = 5;
constexpr std::size_t logs_per_round = 30;
constexpr std::size_t log_rounds = 3;
} // namespace

void test_console_logger(Logger::LogStrategy ls, Logger::LogLevel ll) {
    auto cout_1 = Logger::ConsoleLogger(buffer_size, ls);
    logger_test_helper(cout_1, log_rounds, logs_per_round, ll);
}

void test_file_logger(Logger::LogStrategy ls, Logger::LogLevel ll) {
    auto filelog_1 = Logger::FileLogger("testlog.log", buffer_size, ls);
    logger_test_helper(filelog_1, log_rounds, logs_per_round, ll);
}

} // namespace Test

auto get_level(std::size_t i) -> Logger::LogLevel {
    switch (i % 3) {
    case 0:
        return Logger::LogLevel::Debug;
    case 1:
        return Logger::LogLevel::Warn;
    case 2:
        return Logger::LogLevel::Error;
    default:
        Logger::LogLevel::Error;
    }
}

// auto main(int  /*argc*/, char*  /*argv*/[]) -> int {
//     constexpr int num_threads_test_console_logger = 3;
//     constexpr int num_threads_test_file_logger = 3;
//
//     std::array<std::thread, num_threads_test_console_logger>
//         console_logger_threads;
//     std::array<std::thread, num_threads_test_file_logger> file_logger_threads;
//
//     constexpr Logger::LogStrategy test_strat = Logger::LogStrategy::Blocking;
//
//     for (std::size_t i = 0; i < num_threads_test_console_logger; ++i) {
//         console_logger_threads[i] =
//             std::thread{&Test::test_console_logger, test_strat, get_level(i)};
//     }
//
//     for (std::size_t i = 0; i < num_threads_test_file_logger; ++i) {
//         file_logger_threads[i] =
//             std::thread{&Test::test_file_logger, test_strat, get_level(i)};
//     }
//
//     for (std::size_t i = 0; i < num_threads_test_console_logger; ++i) {
//         console_logger_threads[i].join();
//     }
//
//     for (std::size_t i = 0; i < num_threads_test_file_logger; ++i) {
//         file_logger_threads[i].join();
//     }
// }

 auto main (int  /*argc*/, char * /*argv*/[]) -> int {
    auto buf = AsyncLogger::create_buffer(98);

    for (std::size_t idx = 0; idx < 50; ++idx) {
        AsyncLogger::log(buf, AsyncLogger::LogLevel::Error,
                         std::to_string(idx));
    }

    AsyncLogger::flush(buf);

    return 0;
}
