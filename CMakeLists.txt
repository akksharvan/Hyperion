cmake_minimum_required(VERSION 3.20)
project(Hyperion VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    message(STATUS "Hyperion: Detected ARM64 Architecture (Apple Silicon)")
    add_compile_definitions(HYPERION_ARCH_ARM64)
else()
    message(STATUS "HYPERION: Detected x86_64 Architecture")
    add_compile_definitions(HYPERION_ARCH_X86)
endif()

add_compile_options(-O3 -march=native -fno-rtti -fno-exceptions -Wall -Wextra -Werror -pthread)

include_directories(src)

set(SOURCES
    src/main.cpp
)

add_executable(Hyperion ${SOURCES})
target_link_libraries(Hyperion PRIVATE atomic)

message(STATUS "Hyperion: Build Configuration Ready.")