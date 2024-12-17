#include "log.hpp"

#include <cstddef>
#include <iostream>
#include <string>

void test_logger(Logging::Logger& logger, std::size_t log_rounds,
                 std::size_t logs_per_round) {
    for (std::size_t rnd = 0; rnd < log_rounds; rnd++) {
        for (std::size_t logidx = 1; logidx <= logs_per_round; logidx++) {
            logger.log(Logging::Logger::LogLevel::Error,
                       std::to_string(rnd * logs_per_round + logidx));
        }
        std::string msg(
            "Done logging entries: " + std::to_string(rnd * logs_per_round) +
            "," + std::to_string((rnd + 1) * logs_per_round) + '\n');
        std::cout << msg;
    }
}

int main() {
    Logging::Logger logger{8};
    test_logger(logger, 3, 9);
}
