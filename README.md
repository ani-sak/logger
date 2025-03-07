# Logger

Simple, portable and "asynchronous" C++17 logging library. Performs logging based operations in
seperate thread to minimize load on working threads.

## Dependencies
- [fmt](https://github.com/fmtlib/fmt)
- C++17

## Usage 
### Building 
The `CMakeLists.txt` file statically builds the logger.

Use the following command to build the logger within your project, where
`LOGGER_PATH` is the path to the folder containing the logger `CMakeLists.txt`
file:
```
add_subdirectory(LOGGER_PATH)
```
To then use the logger in target `target_name`, add the following line to your `CMakeLists.txt`:
```
target_link_libraries(target_name PRIVATE async_logger)
```

### Examples
The API header is located in the `include` directory.

```
#include "async_logger/logger.hpp"

auto console = Logger::ConsoleLogger(); console->log(Logger::LogLevel::Warn,
"This warning message displays on the console");

auto file_log = Logger::FileLogger("logfile.log");
file_log->log(Logger::LogLevel::Error, "This error message is logged to a
file");
```
