#include "logger/logger.hpp"

#include "fmt/base.h"
#include "fmt/color.h"

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <string_view>

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
    const fmt::text_style error_style = fmt::fg(fmt::color::red);
    const fmt::text_style success_style = fmt::fg(fmt::color::green);

    // Basic tests
    {
        const char* term_output_file = "test_basic.txt";
        if (std::freopen(term_output_file, "w", stdout) == nullptr) {
            return 1;
        }
        auto term_output_close_defer = defer([]() { std::fclose(stdout); });

        constexpr std::size_t buffer_size = 100;
        auto buf_term =
            Logger::create_buffer(buffer_size, Logger::default_entry_size);

        std::string str_lvalue{"string lvalue"};
        Logger::log(buf_term, Logger::LogLevel::Debug, str_lvalue);

        Logger::log(buf_term, Logger::LogLevel::Debug,
                    std::string{"string rvalue"});
        Logger::log(buf_term, Logger::LogLevel::Debug, "const char pointer");

        std::string_view str_view = "string view";
        Logger::log(buf_term, Logger::LogLevel::Debug, str_view);

        Logger::flush(buf_term);

        // ---------------------------------------------------------------------
        // reset stdout back to the terminal
        if (std::freopen("/dev/tty", "w", stdout) == nullptr) {
            return 1;
        }
        std::ifstream ifs(term_output_file);
        if (!ifs) {
            return 1;
        }

        std::string line;

        std::size_t type_count = 0;
        constexpr std::size_t basic_tests_num = 4;
        while (type_count < basic_tests_num && std::getline(ifs, line)) {
            switch (type_count) {
            case 0: {
                if (line != "string lvalue") {
                    fmt::print(fmt::format(error_style,
                                           "Basic test [{}/{}] failed \n",
                                           type_count + 1, basic_tests_num));
                    return 1;
                }

                fmt::print(fmt::format(success_style,
                                       "Basic test [{}/{}] passed \n",
                                       type_count + 1, basic_tests_num));
                break;
            }
            case 1: {
                if (line != "string rvalue") {
                    fmt::print(fmt::format(error_style,
                                           "Basic test [{}/{}] failed \n",
                                           type_count + 1, basic_tests_num));
                    return 1;
                }

                fmt::print(fmt::format(success_style,
                                       "Basic test [{}/{}] passed \n",
                                       type_count + 1, basic_tests_num));
                break;
            }
            case 2: {
                if (line != "const char pointer") {
                    fmt::print(fmt::format(error_style,
                                           "Basic test [{}/{}] failed \n",
                                           type_count + 1, basic_tests_num));
                    return 1;
                }

                fmt::print(fmt::format(success_style,
                                       "Basic test [{}/{}] passed \n",
                                       type_count + 1, basic_tests_num));
                break;
            }
            case 3: {
                if (line != "string view") {
                    fmt::print(fmt::format(error_style,
                                           "Basic test [{}/{}] failed \n",
                                           type_count + 1, basic_tests_num));
                    return 1;
                }

                fmt::print(fmt::format(success_style,
                                       "Basic test [{}/{}] passed \n",
                                       type_count + 1, basic_tests_num));
                break;
            }
            default: {
                fmt::print(error_style, "Basic test entry {} missing \n",
                           type_count + 1, basic_tests_num, type_count);
                break;
            }
            }

            ++type_count;
        }
    }

    // Inorder/Overwrite tests
    {
        const char* term_output_file = "test_inorder_overwrite.txt";
        if (std::freopen(term_output_file, "w", stdout) == nullptr) {
            return 1;
        }
        auto term_output_close_defer = defer([]() { std::fclose(stdout); });

        constexpr std::size_t buffer_size = 100;
        auto buf_term =
            Logger::create_buffer(buffer_size, Logger::default_entry_size);

        // Check that buffered logs are successfully written in order.
        for (std::size_t idx = 0; idx < buffer_size; ++idx) {
            Logger::log(buf_term, Logger::LogLevel::Debug, std::to_string(idx));
        }
        Logger::flush(buf_term);

        // Verify buffer not overwritten
        for (std::size_t idx = 0; idx < buffer_size * Logger::default_entry_size * 10; ++idx) {
            Logger::log(buf_term, Logger::LogLevel::Debug, std::to_string(idx));
        }
        Logger::flush(buf_term);

        // ---------------------------------------------------------------------
        // reset stdout back to the terminal
        if (std::freopen("/dev/tty", "w", stdout) == nullptr) {
            return 1;
        }
        std::ifstream ifs(term_output_file);
        if (!ifs) {
            return 1;
        }

        std::string line;

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
        while (count < buffer_size * Logger::default_entry_size * 10 &&
               std::getline(ifs, line)) {
            if (line != std::to_string(count)) {
                fmt::print(error_style,
                           "Buffer does not overwrite test failed \n", count);
                fmt::print(line);
                return 1;
            }
            ++count;
        }
        fmt::print(fmt::format(success_style,
                               "Buffer does not overwrite test passed \n"));
    }

    // Log level test
    {
        const char* term_output_file = "test_log_level.txt";
        if (std::freopen(term_output_file, "w", stdout) == nullptr) {
            return 1;
        }
        auto term_output_close_defer = defer([]() { std::fclose(stdout); });

        constexpr std::size_t buffer_size = 100;
        auto *buf_term_ll =
            Logger::create_buffer(buffer_size, Logger::default_entry_size);

        // Verify setting log level is adhered to
        {
            Logger::set_log_level(Logger::LogLevel::Warn);

            Logger::log(buf_term_ll, Logger::LogLevel::Debug, "NO SHOW");
            Logger::log(buf_term_ll, Logger::LogLevel::Warn, "SHOW");
            Logger::log(buf_term_ll, Logger::LogLevel::Error, "SHOW");
        }
        {
            Logger::set_log_level(Logger::LogLevel::Error);

            Logger::log(buf_term_ll, Logger::LogLevel::Debug, "NO SHOW");
            Logger::log(buf_term_ll, Logger::LogLevel::Warn, "NO SHOW");
            Logger::log(buf_term_ll, Logger::LogLevel::Error, "SHOW");
        }
        Logger::flush(buf_term_ll);


        // ---------------------------------------------------------------------
        // reset stdout back to the terminal
        if (std::freopen("/dev/tty", "w", stdout) == nullptr) {
            return 1;
        }
        std::ifstream ifs(term_output_file);
        if (!ifs) {
            return 1;
        }

        constexpr std::size_t set_log_level_warn_test_entries = 2;
        constexpr std::size_t set_log_level_error_test_entries = 1;

        std::string ansi_reset = "\033[0m";
        std::string ansi_rgb_prefix = "\033[38;2;";
        std::string ansi_rgb_red = "255;000;000";
        std::string ansi_rgb_yellow = "255;255;000";
        std::string ansi_cmd_posfix = "m";

        std::string show_msg = "SHOW";

        std::size_t count = 0;
        std::string line;
        while (count < set_log_level_warn_test_entries &&
               std::getline(ifs, line)) {
            std::string ver = ansi_rgb_prefix + ansi_rgb_yellow +
                              ansi_cmd_posfix + show_msg;
            if (count >= 1) {
                ver = ansi_reset + ansi_rgb_prefix + ansi_rgb_red +
                      ansi_cmd_posfix + show_msg;
            }
            if (line != ver) {
                fmt::print(error_style,
                           "Set log level WARN adhered test [{}/{}] failed \n",
                           (count + 1), set_log_level_warn_test_entries);
                return 1;
            }
            ++count;
        }
        fmt::print(fmt::format(success_style,
                               "Set log level WARN adhered test passed \n"));

        count = 0;
        while (count < set_log_level_error_test_entries &&
               std::getline(ifs, line)) {
            std::string ver = ansi_reset + ansi_rgb_prefix + ansi_rgb_red +
                              ansi_cmd_posfix + show_msg;
            if (line != ver) {
                fmt::print(error_style,
                           "Set log level ERROR adhered test failed \n", count);
                return 1;
            }
            ++count;
        }
        fmt::print(fmt::format(success_style,
                               "Set log level ERROR adhered test passed \n"));
    }


    // Color codes test
    {
        const char* term_output_file = "test_color_codes.txt";
        if (std::freopen(term_output_file, "w", stdout) == nullptr) {
            return 1;
        }
        auto term_output_close_defer = defer([]() { std::fclose(stdout); });

        constexpr std::size_t buffer_size = 100;
        auto buf_term =
            Logger::create_buffer(buffer_size, Logger::default_entry_size);

        Logger::set_log_level(Logger::LogLevel::Debug);
        // Verify correct ANSI codes to color error/warn logs
        std::string debug_yellow_msg = "Warn Yellow";
        const char* error_red_msg = "Error Red";
        Logger::log(buf_term, Logger::LogLevel::Warn, debug_yellow_msg);
        Logger::log(buf_term, Logger::LogLevel::Error, error_red_msg);
        Logger::flush(buf_term);

        // ---------------------------------------------------------------------
        // reset stdout back to the terminal
        if (std::freopen("/dev/tty", "w", stdout) == nullptr) {
            return 1;
        }
        std::ifstream ifs(term_output_file);
        if (!ifs) {
            return 1;
        }

        std::string ansi_reset = "\033[0m";
        std::string ansi_rgb_prefix = "\033[38;2;";
        std::string ansi_rgb_red = "255;000;000";
        std::string ansi_rgb_yellow = "255;255;000";
        std::string ansi_cmd_posfix = "m";

        std::string line;
        if (std::getline(ifs, line)) {
            std::string ver = ansi_rgb_prefix + ansi_rgb_yellow +
                              ansi_cmd_posfix + debug_yellow_msg;
            if (line != ver) {
                fmt::print(error_style,
                           "Display WARN in terminal test failed \n");
                return 1;
            }
            fmt::print(fmt::format(success_style,
                                   "Display WARN in terminal test passed \n"));
        }
        if (std::getline(ifs, line)) {
            std::string ver = ansi_reset + ansi_rgb_prefix + ansi_rgb_red +
                              ansi_cmd_posfix + error_red_msg;
            if (line != ver) {
                fmt::print(error_style,
                           "Display ERROR in terminal test failed \n");
                return 1;
            }
            fmt::print(fmt::format(success_style,
                                   "Display ERROR in terminal test passed \n"));
        }

    }

    // ---------------------------------------------------------------------
    // reset stdout back to the terminal
    if (std::freopen("/dev/tty", "w", stdout) == nullptr) {
        return 1;
    }
    fmt::print(fmt::format(success_style, "ALL TESTS PASSED \n"));



    // TODO
    // Verify flush is threadsafe

    // // Test user allocated buffer
    // constexpr std::size_t client_bufsize_bytes = 10240;
    // void* client_buf = malloc(client_bufsize_bytes);
    // auto buf_term_client_alloc =
    //     Logger::create_buffer(client_buf, client_bufsize_bytes);
    // // Run all tests on client allocated buffer

    return 0;
}
