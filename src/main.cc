#include "log.hpp"
#include "ringbuffer.hpp"

#include <chrono>
#include <cstddef>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

// void create_log(RingBuffer<std::string>& rb) {
//     rb.push("1");
//     rb.push("2");
//     rb.push("3");
//     rb.push("4");
//     rb.push("5");
//     rb.push("6");
//     rb.push("7");
//     rb.push("8");
//     rb.push("9");
//     rb.push("10");
//     std::cout << "Done pushing first" << '\n';
//
//     std::this_thread::sleep_for(std::chrono::seconds(5));
//
//     rb.push("11");
//     rb.push("12");
//     rb.push("13");
//     rb.push("14");
//     rb.push("15");
//     rb.push("16");
//     rb.push("17");
//     rb.push("18");
//     rb.push("19");
//     rb.push("20");
//     rb.push("21");
//     std::cout << "Done pushing second" << '\n';
// }
//
// void flush_log(RingBuffer<std::string> &rb) {
//     std::this_thread::sleep_for(std::chrono::seconds(2));
//
//     std::size_t num_msg_to_print = 9;
//     while (num_msg_to_print > 0) {
//         RingBuffer<std::string>::Result res = rb.pop();
//         if (!res.err()) {
//             std::cout << res.data() << '\n';
//             num_msg_to_print--;
//         }
//     }
//
//     std::this_thread::sleep_for(std::chrono::seconds(5));
//
//     num_msg_to_print = 5;
//     while (num_msg_to_print > 0) {
//         RingBuffer<std::string>::Result res = rb.pop();
//         if (!res.err()) {
//             std::cout << res.data() << '\n';
//             num_msg_to_print--;
//         }
//     }
// }
//
// int main() {
//
//     std::cout << "Starting log" << '\n';
//
//     RingBuffer<std::string> rb(9);
//     std::thread a(create_log, std::ref(rb));
//     std::thread b(flush_log, std::ref(rb));
//
//     a.join();
//     b.join();
//
//     return 0;
// }
//

void test_logger(Logging::Logger& logger) {
    logger.log(Logging::Logger::LogLevel::Warn, "1");
    logger.log(Logging::Logger::LogLevel::Warn, "2");
    logger.log(Logging::Logger::LogLevel::Warn, "3");
    logger.log(Logging::Logger::LogLevel::Warn, "4");
    logger.log(Logging::Logger::LogLevel::Warn, "5");
    logger.log(Logging::Logger::LogLevel::Warn, "6");
    logger.log(Logging::Logger::LogLevel::Warn, "7");
    logger.log(Logging::Logger::LogLevel::Warn, "8");
    logger.log(Logging::Logger::LogLevel::Warn, "9");
    logger.log(Logging::Logger::LogLevel::Warn, "10");
    logger.log(Logging::Logger::LogLevel::Warn, "11");
    logger.log(Logging::Logger::LogLevel::Warn, "12");
    logger.log(Logging::Logger::LogLevel::Warn, "13");
    logger.log(Logging::Logger::LogLevel::Warn, "14");
    logger.log(Logging::Logger::LogLevel::Warn, "15");
    logger.log(Logging::Logger::LogLevel::Warn, "16");

    std::cout << "Done storing logs first" << '\n';

    std::this_thread::sleep_for(std::chrono::seconds(3));

    logger.log(Logging::Logger::LogLevel::Warn, "17");
    logger.log(Logging::Logger::LogLevel::Warn, "18");
    logger.log(Logging::Logger::LogLevel::Warn, "19");
    logger.log(Logging::Logger::LogLevel::Warn, "20");
    logger.log(Logging::Logger::LogLevel::Warn, "21");
    logger.log(Logging::Logger::LogLevel::Warn, "22");
    logger.log(Logging::Logger::LogLevel::Warn, "23");
    logger.log(Logging::Logger::LogLevel::Warn, "24");
    logger.log(Logging::Logger::LogLevel::Warn, "25");
    logger.log(Logging::Logger::LogLevel::Warn, "26");
    logger.log(Logging::Logger::LogLevel::Warn, "27");
    logger.log(Logging::Logger::LogLevel::Warn, "28");
    logger.log(Logging::Logger::LogLevel::Warn, "29");

    std::cout << "Done storing logs second" << '\n';

    std::this_thread::sleep_for(std::chrono::seconds(8));

    std::cout << "Exiting" << '\n';
}

int main() {
    // Logging::Logger logger(10);
    Logging::Logger logger;
    test_logger(logger);
}
