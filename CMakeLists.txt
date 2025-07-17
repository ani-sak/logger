cmake_minimum_required(VERSION 3.24)
project(Logger VERSION 0.2.1)

add_library(logger STATIC)

target_sources(logger
	PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/src/logger.cc
)

target_include_directories(logger
	PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
	PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
)

# [LIBRARIES]
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/fmt)
find_package(Threads REQUIRED)

target_link_libraries(logger
	PRIVATE
	fmt::fmt
	Threads::Threads
)

target_compile_features(logger
	PRIVATE
	cxx_std_17
)

option(LOGGER_BUILD_TESTS "Build tests")
if (${LOGGER_BUILD_TESTS})
	message(STATUS "Building logger tests")

	add_executable(logger_test)

	target_sources(logger_test
		PRIVATE
		${CMAKE_CURRENT_LIST_DIR}/test/main.cc
	)

	target_link_libraries(logger_test
		PRIVATE 
		logger 
		fmt::fmt
	)
endif()
