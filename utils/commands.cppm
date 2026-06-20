export module commands;

export constexpr char command_sync_pc = 0x4F;  // PC will send this bit to sync with mcu
export constexpr char command_sync_mcu = 0x4E; // MCU will send this bit to sync with pc
export constexpr char command_err = 0x4D;
export constexpr char command_ok = 0x4C;
export constexpr char command_start_byte = 0x4B;
export constexpr char command_end_byte = 0x4A;
export constexpr char command_get_addr = 0x49;
export constexpr char command_sign_transaction = 0x48;
export constexpr char command_show_addr = 0x47;
export constexpr char command_finish = 0x46;
