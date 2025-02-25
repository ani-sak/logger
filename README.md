# Logger

Simple, portable and "asynchronous" C++17 logging library. Performs logging based operations in
seperate thread to minimize load on working threads.

## Dependencies
- [fmt](https://github.com/fmtlib/fmt)
- C++17

## Usage 
### Building 
The CMake file statically builds the logger. To add it to
your project simply add the following line to your `CMakeLists.txt':
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
