cmake_minimum_required(VERSION 3.20)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

project(Limit_Order_Book LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)

add_executable(MatchingEngine src/main.cpp)
add_library(lob STATIC src/engine.cpp)

target_include_directories(lob PUBLIC include)
target_link_libraries(lob PUBLIC Threads::Threads)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(lob PRIVATE -Wall -Wextra -Wpedantic -Werror -O3)
    target_compile_options(MatchingEngine PRIVATE -Wall -Wextra -Wpedantic -Werror -O3)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(lob PRIVATE /W4 /permissive- /WX /O2)
    target_compile_options(MatchingEngine PRIVATE /W4 /permissive- /WX /O2)
endif()

target_link_libraries(MatchingEngine PRIVATE lob)

add_subdirectory(benchmarks)