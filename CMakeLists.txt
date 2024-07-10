cmake_minimum_required(VERSION 3.22)

project(s0m4b0dY_parallel_algorithms)

find_package(Boost COMPONENTS thread system REQUIRED)

add_subdirectory(../utils-library ./build/utils)

get_target_property(${s0m4b0dY_utils_lib}_INCLUDES ${s0m4b0dY_utils_lib} INCLUDE_DIRECTORIES)


set(GTEST_EXECUTABLE ON CACHE BOOL "Build gtest executable?")

set(main_lib "s0m4b0dY_parallel_algorithms_threading")

set(COMMON_SOURCE_FILES
    src/s0_thread_pool.cpp
)

set(COMMON_PUBLIC_INCLUDES
    include/public
    ${${s0m4b0dY_utils_lib}_INCLUDES}
)

set(COMMON_PRIVATE_INCLUDES
    include/private
)

set(COMMON_LIBS ${s0m4b0dY_utils_lib} Boost::system Boost::thread)

if (GTEST_EXECUTABLE)
    include(cmake/build_test_executable.cmake)
endif()

add_library(${main_lib})

target_include_directories(${main_lib}
    PUBLIC ${COMMON_PUBLIC_INCLUDES}
    PRIVATE ${COMMON_PRIVATE_INCLUDES}
)

target_link_libraries(${main_lib} PUBLIC ${COMMON_LIBS})

target_sources(${main_lib} # Header only?
    PRIVATE ${COMMON_SOURCE_FILES}
)

target_compile_features(${main_lib} INTERFACE cxx_std_20)
