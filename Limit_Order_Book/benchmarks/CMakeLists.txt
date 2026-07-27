cmake_minimum_required(VERSION 3.14)
project(EngineBenchmarks LANGUAGES CXX)

find_package(Threads REQUIRED)

include(FetchContent)

FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        v1.9.0 
)

set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(benchmark)

add_executable(engine_benchmark engine_bench.cpp)

target_link_libraries(engine_benchmark PRIVATE lob benchmark::benchmark_main Threads::Threads)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(engine_benchmark PRIVATE -O3 -march=native -Wall -Wextra -Wpedantic -Werror)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(engine_benchmark PRIVATE /O2 /W4 /WX)
endif()