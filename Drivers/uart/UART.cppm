module;

#include <span>
#include <cstring>
#include <optional>

#include "main.h"
export module uart;

import commands;
import result;
import logger_mcu;


constexpr uint8_t len_msg_size = 6;
export class UART {
    UART_HandleTypeDef& huart1;

    void sync() const {
        uint8_t data = 0;

        while (data != command_sync_pc) {
            HAL_UART_Receive(&huart1, &data, 1, 100);
        }
        constexpr uint8_t command = static_cast<uint8_t>(command_sync_mcu);

        HAL_UART_Transmit(&huart1, &command, 1, 100);
    }

    [[nodiscard]] Result test() const {
        uint8_t data = 0;

        Result result;

        HAL_UART_Receive(&huart1, &data, 1, 100);
        if (data == command_ok)
            result = Result::Ok;
        else
            result = Result::Err;

        constexpr uint8_t command = static_cast<uint8_t>(command_sync_mcu);

        HAL_UART_Transmit(&huart1, &command, 1, 100);
        return result;
    }

    void send_res(Result const res) const {
        uint8_t data = 0;
        uint8_t const result = res == Result::Ok ? command_ok : command_err;
        while (data != command_sync_pc) {
            HAL_UART_Transmit(&huart1, &result, 1, 100);
            HAL_UART_Receive(&huart1, &data, 1, 100);
        }
    }

public:
    UART(UART_HandleTypeDef& huart1) : huart1(huart1){}

    [[nodiscard]] Result send(std::span<const char> const msg) const {
        // 1 start byte | 4 data bytes | 1 end byte
        uint8_t len_msg[len_msg_size];
        len_msg[0] = command_start_byte;
        uint32_t const msg_size = static_cast<uint32_t>(msg.size());

        std::memcpy(len_msg + 1, &msg_size, 4);
        len_msg[len_msg_size - 1] = command_end_byte;

        uint32_t const data_msg_size = msg_size + 2;
        uint8_t data_msg[data_msg_size];

        // 1 start byte | data | 1 end byte
        data_msg[0] = command_start_byte;
        std::memcpy(data_msg + 1, msg.data(), msg.size());
        data_msg[data_msg_size - 1] = command_end_byte;

        uint8_t attempts = 0;

        sync();
        do {
            if (HAL_UART_Transmit(&huart1, len_msg, len_msg_size, 100) != HAL_OK)
                LoggerMCU::exception();
            if (++attempts >= 3)
                return Result::Err;
        }
        while (test() != Result::Ok);

        attempts = 0;
        sync();
        do {
            HAL_UART_Transmit(&huart1, data_msg, data_msg_size, 100);
            if (++attempts >= 3)
                return Result::Err;
        }
        while (test() != Result::Ok);

        return Result::Ok;
    }

    /// @brief always use this function
    /// @return return received message length
    [[nodiscard]] std::optional<uint32_t> receive_len() const {
        uint8_t len_msg_buf[len_msg_size];
        uint8_t attempts = 0;

        sync();
        while (true) {
            HAL_UART_Receive(&huart1, len_msg_buf, len_msg_size, 100);
            if (++attempts >= 3)
                return std::nullopt;
            if (len_msg_buf[0] == command_start_byte && len_msg_buf[len_msg_size - 1] == command_end_byte) {
                send_res(Result::Ok);
                break;
            }
            else
                send_res(Result::Err);
        }

        uint32_t len_msg;
        std::memcpy(&len_msg, len_msg_buf + 1, 4);

        return len_msg;
    }

    [[nodiscard]] Result receive(std::span<char> buf) const {
        uint8_t attempts = 0;

        uint32_t msg_size = static_cast<uint32_t>(buf.size()) + 2;
        uint8_t msg[msg_size];

        sync();
        while (true) {
            HAL_UART_Receive(&huart1, msg, msg_size, 100);

            if (++attempts >= 3)
                return Result::Err;
            if (msg[0] == command_start_byte && msg[msg_size - 1] == command_end_byte) {
                send_res(Result::Ok);
                break;
            }
            else
                send_res(Result::Err);
        }

        std::memcpy(buf.data(), msg + 1, msg_size - 1);

        return Result::Ok;
    }

    [[nodiscard]] std::optional<char> receive_command() const {
        uint32_t len = receive_len().value();

        if (len != 1)
            return std::nullopt;

        char command;
        Result res = receive(std::span(&command, len));

        if (res == Result::Err || command < 0x47 || command > 0x4F)
            return std::nullopt;

        return command;
    }

    [[nodiscard]] Result send_command(char const command) const {
        if (command < 0x46 || command > 0x4F)
            LoggerMCU::exception();
        return send(std::span(&command, 1));
    }
};