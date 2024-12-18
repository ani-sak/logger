cmake_minimum_required(VERSION 3.24)
project(AsyncLogger VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# [SOURCES]
set(SOURCE_PREFIX ${CMAKE_CURRENT_LIST_DIR}/src)
set(SRC ${SOURCE_PREFIX}/log.cc)
set(SRC_MAIN ${SOURCE_PREFIX}/main.cc)

# [INCLUDES]
set(INCL ${CMAKE_CURRENT_LIST_DIR}/src)

# [LIBRARIES]
set(LIBRARY_PREFIX ${CMAKE_CURRENT_LIST_DIR}/lib)

add_subdirectory(${LIBRARY_PREFIX}/fmt)

# [TARGETS]
add_executable(async_logger ${SRC} ${SRC_MAIN})
target_include_directories(async_logger PRIVATE ${INCL})
target_link_libraries(async_logger PRIVATE pthread fmt::fmt)
