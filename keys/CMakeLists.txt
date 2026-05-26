cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_C_COMPILER "clang")

project("keys" CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

find_package(OpenSSL REQUIRED)

add_executable(keys main.cpp)

target_sources(keys PRIVATE
    FILE_SET CXX_MODULES FILES
    wallet.cppm
)

target_include_directories(keys PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/wallet-core/include"
)

target_link_libraries(keys PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/wallet-core/build/libTrustWalletCore.a"
    "${CMAKE_CURRENT_SOURCE_DIR}/wallet-core/build/libTrezorCrypto.a"
    "${CMAKE_CURRENT_SOURCE_DIR}/wallet-core/build/libprotobuf.a"
    OpenSSL::Crypto
    c++
    c++abi
)