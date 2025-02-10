# Logger

Simple C++ logging library that minimizes time spent on work threads, 
and performs logging operations in a seperate thread.

## Dependencies
- [fmt](https://github.com/fmtlib/fmt?tab=License-1-ov-file#readme)

## Usage
### Build
The CMake file statically builds the logger. To add it to your project simply add the following line to your `CMakeLists.txt'
```
target_link_libraries(async_logger_test PRIVATE async_logger fmt::fmt)
```

### Code
The API header is located in the `include` directory.



