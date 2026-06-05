module;

#include <cstdint>

export module commands;

export enum class Commands : uint8_t{
    SYNC_PC = 0xFF,  // PC will send this bit to sync with mcu
    SYNC_MCU = 0xEE, // MCU will send this bit to sync with pc
    GET_ADDR = 0xDD,
    SIGN_TRANSACTION = 0xCC,
    SHOW_ADDR = 0xBB
};