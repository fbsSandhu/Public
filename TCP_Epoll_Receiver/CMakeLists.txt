cmake_minimum_required(VERSION 3.20)

project(LFTCP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)

add_executable(engine src/main.cpp src/tcp_receiver.cpp)
target_include_directories(engine PRIVATE include)
target_link_libraries(engine PRIVATE Threads::Threads)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(engine PRIVATE -Wall -Wextra -Wpedantic -Werror -O3 -march=native)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(engine PRIVATE /W4 /permissive- /WX /O2 -march=native)
endif()


add_custom_target(run_all
    COMMAND $<TARGET_FILE:engine> &
    COMMAND sleep 1
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/testing/test.py
)


