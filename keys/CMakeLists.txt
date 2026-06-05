cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_C_COMPILER "clang")

set (CMAKE_PROJECT_NAME keys)

project(${CMAKE_PROJECT_NAME} CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

find_package(OpenSSL REQUIRED)

add_executable(${CMAKE_PROJECT_NAME} main.cpp)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    FILE_SET CXX_MODULES FILES
    wallet.cppm
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../wallet-core/include"
)

target_link_libraries(keys PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../wallet-core/build/libTrustWalletCore.a"
    "${CMAKE_CURRENT_SOURCE_DIR}/../wallet-core/build/libTrezorCrypto.a"
    "${CMAKE_CURRENT_SOURCE_DIR}/../wallet-core/build/libprotobuf.a"
    OpenSSL::Crypto
    c++
    c++abi
)