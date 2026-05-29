cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_C_COMPILER "clang")

set(CMAKE_PROJECT_NAME main_pc)
project(${CMAKE_PROJECT_NAME} CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

include(FetchContent)
FetchContent_Declare(
        asio
        GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
)
FetchContent_MakeAvailable(asio)

add_executable(${CMAKE_PROJECT_NAME} main.cpp)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    FILE_SET CXX_MODULES FILES
    serial.cppm
)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${asio_SOURCE_DIR}/asio/include
)

find_package(Threads REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Threads::Threads)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        ASIO_STANDALONE
        PROJECT_WORKING_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)