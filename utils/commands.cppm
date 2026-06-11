export module commands;

typedef unsigned char uint8_t;

export constexpr uint8_t command_sync_pc = 0xFF;  // PC will send this bit to sync with mcu
export constexpr uint8_t command_sync_mcu = 0xEE; // MCU will send this bit to sync with pc
export constexpr uint8_t command_err = 0xDD;
export constexpr uint8_t command_ok = 0xCC;
export constexpr uint8_t command_start_byte = 0xBB;
export constexpr uint8_t command_end_byte = 0xAA;
export constexpr uint8_t command_get_addr = 0x99;
export constexpr uint8_t command_sign_transaction = 0x88;
export constexpr uint8_t command_show_addr = 0x77;
