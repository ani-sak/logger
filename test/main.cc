#include "logger/logger.hpp"

#include "fmt/base.h"
#include "fmt/color.h"

#include <fstream>
#include <string>

namespace {
template <typename F>
class Defer {
public:
    Defer(F fxn) : fxn(fxn) {}
    Defer(Defer&&) = delete;
    Defer(const Defer&) = delete;
    auto operator=(Defer&&) -> Defer& = delete;
    auto operator=(const Defer&) -> Defer& = delete;
    ~Defer() { fxn(); }

    F fxn;
};

template <typename F>
auto defer(F func) -> Defer<F> {
    return Defer<F>{func};
}
} // namespace

auto main(int /*argc*/, char* /*argv*/[]) -> int {
    const char* term_output_file = "test_terminal.txt";

    constexpr std::size_t buffer_size = 100;
    // Terminal logging APIs redirected to term_output_file for verification.
    {

        if (std::freopen(term_output_file, "w", stdout) == nullptr) {
            return 1;
        }
        auto term_output_close_defer = defer([]() { std::fclose(stdout); });

        auto buf_term = AsyncLogger::create_buffer(
            buffer_size, AsyncLogger::default_entry_size);

        // Test all basic log string types are supported
        std::string str_lvalue{"string lvalue"};
        AsyncLogger::log(buf_term, AsyncLogger::LogLevel::Debug, str_lvalue);

        AsyncLogger::log(buf_term, AsyncLogger::LogLevel::Debug,
                         std::string{"string rvalue"});
        AsyncLogger::log(buf_term, AsyncLogger::LogLevel::Debug,
                         "const char pointer");

        AsyncLogger::flush(buf_term);

        // Check that buffered logs are successfully written in order.
        for (std::size_t idx = 0; idx < buffer_size; ++idx) {
            AsyncLogger::log(buf_term, AsyncLogger::LogLevel::Debug,
                             std::to_string(idx));
        }

        AsyncLogger::flush(buf_term);

        // Verify buffer not overwritten
        for (std::size_t idx = 0; idx < buffer_size * 10; ++idx) {
            AsyncLogger::log(buf_term, AsyncLogger::LogLevel::Debug,
                             std::to_string(idx));
        }

        AsyncLogger::flush(buf_term);

        // Verify setting log level is adhered to
        {
            AsyncLogger::set_log_level(AsyncLogger::LogLevel::Warn);
            auto log_level_reset_defer = defer([]() {
                AsyncLogger::set_log_level(AsyncLogger::LogLevel::Debug);
            });

            AsyncLogger::log(buf_term, AsyncLogger::LogLevel::Debug, "NO SHOW");
            AsyncLogger::log(buf_term, AsyncLogger::LogLevel::Warn, "SHOW");
            AsyncLogger::log(buf_term, AsyncLogger::LogLevel::Error, "SHOW");
        }
        {
            AsyncLogger::set_log_level(AsyncLogger::LogLevel::Error);
            auto log_level_reset_defer = defer([]() {
                AsyncLogger::set_log_level(AsyncLogger::LogLevel::Debug);
            });

            AsyncLogger::log(buf_term, AsyncLogger::LogLevel::Debug, "NO SHOW");
            AsyncLogger::log(buf_term, AsyncLogger::LogLevel::Warn, "NO SHOW");
            AsyncLogger::log(buf_term, AsyncLogger::LogLevel::Error, "SHOW");
        }

        // TODO
        // Verify log entry longer than buffer entry size clipped

        // // Verify log is threadsafe
        // constexpr std::size_t thread_num_total = 10;
        // constexpr std::size_t thread_log_entries =
        //     buffer_size / thread_num_total;
        //
        // std::vector<std::thread> log_thread_vector;
        // log_thread_vector.reserve(thread_num_total);
        //
        // for (std::size_t thread_idx = 0; thread_idx < thread_num_total;
        //      ++thread_idx) {
        //     log_thread_vector.emplace_back([buf_term]() {
        //         for (std::size_t log_entry = 0; log_entry <
        //         thread_log_entries;
        //              ++log_entry) {
        //             AsyncLogger::log(buf_term, AsyncLogger::LogLevel::Debug,
        //                              std::to_string(log_entry));
        //         }
        //     });
        // }

        // TODO
        // Verify flush is threadsafe
    }

    // Verify file has correct entries.
    freopen("/dev/tty", "w", stdout); // reset stdout back to the terminal

    std::ifstream ifs(term_output_file);
    if (!ifs) {
        return 1;
    }

    const fmt::text_style error_style = fmt::fg(fmt::color::red);
    const fmt::text_style success_style = fmt::fg(fmt::color::green);

    std::string line;

    std::size_t type_count = 0;
    while (type_count < 3 && std::getline(ifs, line)) {
        switch (type_count) {
        case 0: {
            if (line != "string lvalue") {
                fmt::print(
                    fmt::format(error_style, "Basic test [1/3] failed \n"));
                return 1;
            }

            fmt::print(
                fmt::format(success_style, "Basic test [1/3] passed \n"));
            break;
        }
        case 1: {
            if (line != "string rvalue") {
                fmt::print(
                    fmt::format(error_style, "Basic test [2/3] failed \n"));
                return 1;
            }

            fmt::print(
                fmt::format(success_style, "Basic test [2/3] passed \n"));
            break;
        }
        case 2: {
            if (line != "const char pointer") {
                fmt::print(
                    fmt::format(error_style, "Basic test [3/3] failed \n"));
                return 1;
            }

            fmt::print(
                fmt::format(success_style, "Basic test [3/3] passed \n"));
            break;
        }
        default: {
            fmt::print(error_style, "Basic test entry {} missing \n",
                       type_count);
            break;
        }
        }

        ++type_count;
    }

    std::size_t count = 0;
    while (count < buffer_size && std::getline(ifs, line)) {
        if (line != std::to_string(count)) {
            fmt::print(error_style, "Inorder test entry {} out of order \n",
                       count);
            return 1;
        }
        ++count;
    }
    fmt::print(fmt::format(success_style, "Inorder test passed \n"));

    count = 0;
    while (count < buffer_size && std::getline(ifs, line)) {
        if (line != std::to_string(count)) {
            fmt::print(error_style, "Buffer does not overwrite test failed \n",
                       count);
            return 1;
        }
        ++count;
    }
    fmt::print(
        fmt::format(success_style, "Buffer does not overwrite test passed \n"));

    constexpr std::size_t set_log_level_warn_test_entries = 2;
    constexpr std::size_t set_log_level_error_test_entries = 1;

    count = 0;
    while (count < set_log_level_warn_test_entries && std::getline(ifs, line)) {
        if (line != "SHOW") {
            fmt::print(error_style, "Set log level adhered test failed \n",
                       count);
            return 1;
        }
        ++count;
    }
    fmt::print(fmt::format(success_style,
                           "Set log level WARN adhered test passed \n"));

    count = 0;
    while (count < set_log_level_error_test_entries &&
           std::getline(ifs, line)) {
        if (line != "SHOW") {
            fmt::print(error_style, "Set log level adhered test failed \n",
                       count);
            return 1;
        }
        ++count;
    }
    fmt::print(fmt::format(success_style,
                           "Set log level ERROR adhered test passed \n"));


    fmt::print(fmt::format(success_style,
                           "\nAll tests passed!\n"));
    return 0;
}
