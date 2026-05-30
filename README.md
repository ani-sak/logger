# Logger

Simple logger which stores logs in packed buffer and allows user to control
when logs are flushed.

## Dependencies
- C++11

## Build
### CMake
To use the logger with a cmake project:
```
add_subdirectory(PATH_TO_LOGGER_LIBRARY)
target_link_libraries(your_target PRIVATE async_logger)
```

### g++
TODO

## Usage

The API is in a single header file at `include/logger/logger.hpp`.
```
// Quickstart
auto logbuf = Logger::create_buffer(); // Create a buffer to store logs

Logger::log(logbuf, Logger::LogLevel::Debug, "Debug message");
Logger::log(logbuf, Logger::LogLevel::Warn, "Warning message"); // Colored yellow
Logger::log(logbuf, Logger::LogLevel::Error, "Error message"); // Colored red

Logger::flush(logbuf); // Write above messages to terminal

Logger::log(logbuf, Logger::LogLevel::Debug, "My log file");
Logger::flush(logbuf, "log_file.txt"); // Write "Debug: My log file" to log_file.txt

Logger::free_buffer(logbuf); // Free the memory allocated by create_buffer
```

The program log level can be modified using `set_log_level`.
Log messages with lower severity than the program log level will not be logged
(or stored in the buffer).
```
auto logbuf = Logger::create_buffer(); // Create a buffer to store logs

Logger::set_log_level(Logger::LogLevel::Error);
Logger::log(logbuf, Logger::LogLevel::Debug, "Debug message"); // Not logged
Logger::log(logbuf, Logger::LogLevel::Warn, "Warning message"); // Not logged
Logger::log(logbuf, Logger::LogLevel::Error, "Error message");

Logger::flush(logbuf); // Write only "Error message" to the terminal
```
The program log level can be modified at any time, and will filter out any
subsequent logs with lower severity.

The memory used by the buffer can be provided by the user:
```
void* memory = malloc(65536);
auto logbuf = Logger::create_buffer(memory, 65536);
```


### TODO
- Add API to automatically flush if buffer is full.
