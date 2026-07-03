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
    GIT_TAG        asio-1-38-0
)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.12.0
)

FetchContent_MakeAvailable(asio nlohmann_json)

add_executable(${CMAKE_PROJECT_NAME} main.cpp)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    FILE_SET CXX_MODULES
    BASE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../utils
    FILES
    serial.cppm
    BEP20.cppm
    input_manager.cppm
    transaction.cppm
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/logger_pc.cppm
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/commands.cppm
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/result.cppm
)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${asio_SOURCE_DIR}/asio/include
    ${nlohmann_json_SOURCE_DIR}/include
)

find_package(Threads REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    Threads::Threads
    nlohmann_json::nlohmann_json
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    ASIO_STANDALONE
    PROJECT_WORKING_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)