cmake_minimum_required(VERSION 3.28)


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/gcc-arm-none-eabi.cmake")

include(FetchContent)
FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG        v3.12.0
)

FetchContent_MakeAvailable(nlohmann_json)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_PROJECT_NAME coldWallet)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

enable_language(C CXX ASM)

add_executable(${CMAKE_PROJECT_NAME}
        Src/main.cpp)

add_subdirectory(cmake/stm32cubemx)

target_compile_features(${CMAKE_PROJECT_NAME} PRIVATE cxx_std_23)

target_sources(${CMAKE_PROJECT_NAME} PUBLIC
        FILE_SET CXX_MODULES FILES
        Drivers/display/display.cppm
        Drivers/display/HAL_Delay_us.cppm
        Drivers/uart/UART.cppm
        utils/logger_mcu.cppm
        utils/result.cppm
        utils/commands.cppm
        utils/transaction.cppm
        Drivers/keys/keys.cppm
        Drivers/button/button.cppm
)

list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/wallet-core/include
)
target_link_libraries(${CMAKE_PROJECT_NAME}
    nlohmann_json::nlohmann_json
    stm32cubemx
    "${CMAKE_CURRENT_SOURCE_DIR}/../wallet-core/build/libTrustWalletCore.a"
    "${CMAKE_CURRENT_SOURCE_DIR}/../wallet-core/build/libTrezorCrypto.a"
    "${CMAKE_CURRENT_SOURCE_DIR}/../wallet-core/build/libprotobuf.a"
)
