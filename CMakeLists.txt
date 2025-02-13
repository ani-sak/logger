cmake_minimum_required(VERSION 3.24)
project(AsyncLogger VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# [SOURCES]
set(SOURCE_PREFIX ${CMAKE_CURRENT_LIST_DIR}/src)
set(SRC 
	${SOURCE_PREFIX}/logger.cc
	${SOURCE_PREFIX}/FileLoggerImpl.cc
)

# [INCLUDES]
set(INCL ${CMAKE_CURRENT_LIST_DIR}/src)
set(LIB_INCL ${CMAKE_CURRENT_LIST_DIR}/include)

# [LIBRARIES]
set(LIBRARY_PREFIX ${CMAKE_CURRENT_LIST_DIR}/lib)

add_subdirectory(${LIBRARY_PREFIX}/fmt)

# [TARGETS]
add_library(async_logger STATIC ${SRC})
target_include_directories(async_logger 
	PRIVATE ${INCL} 
	PUBLIC ${LIB_INCL})
target_link_libraries(async_logger PRIVATE pthread fmt::fmt)

add_executable(async_logger_test ${CMAKE_CURRENT_LIST_DIR}/test/main.cc)
target_link_libraries(async_logger_test PRIVATE async_logger fmt::fmt)
