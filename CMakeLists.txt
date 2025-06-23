cmake_minimum_required(VERSION 3.24)
project(AsyncLogger VERSION 0.0.3)

add_library(async_logger STATIC)

target_sources(async_logger
	PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/src/logger.cc
)

target_include_directories(async_logger
	PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
	PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
)

# [LIBRARIES]
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/fmt)
find_package(Threads REQUIRED)

target_link_libraries(async_logger
	PRIVATE
	fmt::fmt
	Threads::Threads
)

target_compile_features(async_logger
	PRIVATE
	cxx_std_17
)

option(LOGGER_BUILD_TESTS "Build tests")
if (${LOGGER_BUILD_TESTS})
	message(STATUS "Building logger tests")

	add_executable(async_logger_test)

	target_sources(async_logger_test
		PRIVATE
		${CMAKE_CURRENT_LIST_DIR}/test/main.cc
	)

	target_link_libraries(async_logger_test
		PRIVATE 
		async_logger 
		fmt::fmt
	)
endif()
