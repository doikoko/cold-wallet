 module;

#include "main.h"

#include <span>

export module display;

import HAL_Delay_us;

export class Display {
    #define DISPLAY_RS  GPIOB
    #define DISPLAY_RS_PIN  GPIO_PIN_0

    #define DISPLAY_E  GPIOB
    #define DISPLAY_E_PIN  GPIO_PIN_1

    #define DISPLAY_RW  GPIOB
    #define DISPLAY_RW_PIN  GPIO_PIN_2


    #define DISPLAY_DB4  GPIOB
    #define DISPLAY_DB4_PIN  GPIO_PIN_4

    #define DISPLAY_DB5  GPIOB
    #define DISPLAY_DB5_PIN  GPIO_PIN_5

    #define DISPLAY_DB6  GPIOB
    #define DISPLAY_DB6_PIN  GPIO_PIN_6

    #define DISPLAY_DB7  GPIOB
    #define DISPLAY_DB7_PIN  GPIO_PIN_7

    void send_nibble(uint8_t const nibble, bool const is_data) {
        if (is_data)
            HAL_GPIO_WritePin(DISPLAY_RS, DISPLAY_RS_PIN, GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(DISPLAY_RS, DISPLAY_RS_PIN, GPIO_PIN_RESET);

        HAL_GPIO_WritePin(
            DISPLAY_DB4,
            DISPLAY_DB4_PIN,
            nibble & 0x01 ? GPIO_PIN_SET : GPIO_PIN_RESET
        );

        HAL_GPIO_WritePin(
            DISPLAY_DB5,
            DISPLAY_DB5_PIN,
            nibble & 0x02 ? GPIO_PIN_SET : GPIO_PIN_RESET
        );

        HAL_GPIO_WritePin(
            DISPLAY_DB6,
            DISPLAY_DB6_PIN,
            nibble & 0x04 ? GPIO_PIN_SET : GPIO_PIN_RESET
        );

        HAL_GPIO_WritePin(
            DISPLAY_DB7,
            DISPLAY_DB7_PIN,
            nibble & 0x08 ? GPIO_PIN_SET : GPIO_PIN_RESET
        );

        HAL_GPIO_WritePin(DISPLAY_E, DISPLAY_E_PIN, GPIO_PIN_SET);
        HAL_Delay_us(1);
        HAL_GPIO_WritePin(DISPLAY_E, DISPLAY_E_PIN, GPIO_PIN_RESET);
    }

    void set_mode(bool const is_read) {
        if (is_read)
            HAL_GPIO_WritePin(DISPLAY_RW, DISPLAY_RW_PIN, GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(DISPLAY_RW, DISPLAY_RW_PIN, GPIO_PIN_RESET);
    }

    void send_byte(uint8_t const byte, bool const is_data) {
        send_nibble(byte >> 4, is_data);
        send_nibble(byte & 0x0F, is_data);
    }

    void cursor_right_shift() {
        set_mode(false);
        send_byte(0x14, false);
        HAL_Delay_us(37);
    }

public:
    void clear_display() {
        set_mode(false);
        send_byte(1, false);
        HAL_Delay(2);
    }

    void return_cursor() {
        set_mode(false);
        send_byte(2, false);
        HAL_Delay(2);
    }

    void write_to_dram(uint8_t const data) {
        set_mode(true);
        send_byte(data, true);
        HAL_Delay_us(37);
    }

    /// @param is_increment true - cursor moves right, false - left
    /// @param is_display true - display shift, false - cursor shift
    void entry_mode_set(bool const is_increment, bool const is_display) {
        uint8_t command = 0x04;

        if (is_increment)
            command |= 0x02;
        if (is_display)
            command |= 0x01;

        set_mode(false);
        send_byte(command, false);
        HAL_Delay_us(37);
    }

    void print(std::span<const char> const text) {
        for (const char& ch : text) {
            write_to_dram(ch);
            cursor_right_shift();
        }
    }
};