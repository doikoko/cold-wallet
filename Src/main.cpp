#include <span>
#include <optional>
#include <string>

#include "main.h"
#include "stm32f4xx_hal_gpio.h"

import display;
import logger_mcu;
import uart;
import result;
import commands;
import keys;
import button;
import signer;

uint16_t find_device(I2C_HandleTypeDef& hi2c1) {
    for (uint16_t addr = 1; addr < 128; addr++) {
        if (HAL_I2C_IsDeviceReady(&hi2c1, addr << 1, 1, 100) == HAL_OK) {
            return addr;
        }
    }
    return 0xFF'FF;
}

extern "C" void main_cpp(I2C_HandleTypeDef& hi2c1, UART_HandleTypeDef& huart1) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

    uint16_t const addr = find_device(hi2c1);
    if (addr == 0xFF'FF)
        LoggerMCU::exception();

    Display display(hi2c1, addr, 2);
    UART const uart(huart1);
    Keys const keys;
    Button const button(GPIOA, 0);

    while (true) {
        auto command = uart.receive_command();

        if (!command.has_value())
            LoggerMCU::exception();

        switch (command.value()) {
            case command_get_addr: {
                if (uart.send(keys.addr) == Result::Err)
                    LoggerMCU::exception();
            }
            case command_show_addr: {
                display.print("compare addresses");
                while (!button.is_pressed()){}
                display.print(keys.addr);
                while (!button.is_pressed()){}
                if (uart.send_command(command_finish) == Result::Err)
                    LoggerMCU::exception();
            }
            case command_sign_transaction: {
                std::optional len_opt = uart.receive_len();
                if (!len_opt.has_value()) {
                    display.print("receiving len error");
                    LoggerMCU::exception();
                }

                uint32_t len = len_opt.value();
                char buf[len] = {};

                if (uart.receive(std::span(buf, len)) == Result::Err) {
                    display.print("receiving transaction error");
                    LoggerMCU::exception();
                }

                display.print("confirm transaction");
                while (!button.is_pressed()){}
                constexpr uint8_t chain_id = 0x38;
                Signature signature = sign_transaction(
                    reinterpret_cast<char*>(buf),
                    keys.private_key,
                    chain_id
                );

                Result const res = uart.send(std::span<const char>(
                    reinterpret_cast<const char*>(signature.r.data()),
                    signature.r.size()
                ));

                if (res == Result::Err)
                    LoggerMCU::exception();
            }
            default: {
                LoggerMCU::exception();
            }
        }
    }
}