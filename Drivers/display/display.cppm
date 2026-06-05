module;

#include <span>

#include "main.h"

export module display;

import HAL_Delay_us;
import logger_mcu;

namespace lcd_commands {
    constexpr uint8_t CLEAR_DISPLAY = 0x01;
    constexpr uint8_t RETURN_HOME = 0x02;
    constexpr uint8_t ENTRY_MODE_SET = 0x04;
    constexpr uint8_t DISPLAY_CONTROL = 0x08;
    constexpr uint8_t CURSOR_SHIFT = 0x10;
    constexpr uint8_t FUNCTION_SET = 0x20;
    constexpr uint8_t SET_CGRAM_ADDR = 0x40;
    constexpr uint8_t SET_DDRAM_ADDR = 0x80;
}

namespace lcd_entry_mode_flags {
    constexpr uint8_t ENTRY_RIGHT = 0x00;
    constexpr uint8_t ENTRY_LEFT = 0x02;
    constexpr uint8_t ENTRY_SHIFT_INCREMENT = 0x01;
    constexpr uint8_t ENTRY_SHIFT_DECREMENT = 0x00;
}

namespace lcd_on_of_control_flags {
    constexpr uint8_t DISPLAY_ON = 0x04;
    constexpr uint8_t DISPLAY_OFF = 0x00;
    constexpr uint8_t CURSOR_ON = 0x02;
    constexpr uint8_t CURSOR_OFF = 0x00;
    constexpr uint8_t BLINK_ON = 0x01;
    constexpr uint8_t BLINK_OFF = 0x00;
}

namespace lcd_cursor_shift {
    constexpr uint8_t DISPLAY_MOVE = 0x08;
    constexpr uint8_t CURSOR_MOVE = 0x00;
    constexpr uint8_t MOVE_RIGHT = 0x04;
    constexpr uint8_t MOVE_LEFT = 0x00;
}

namespace lcd_function_set {
    constexpr uint8_t LCD_8_BIT_MODE = 0x10;
    constexpr uint8_t LCD_4_BIT_MODE = 0x00;
    constexpr uint8_t LCD_2_LINE = 0x08;
    constexpr uint8_t LCD_1_LINE = 0x00;
    constexpr uint8_t LCD_5x10_DOTS = 0x04;
    constexpr uint8_t LCD_5x8_DOTS = 0x00;
}

namespace lcd_blacklight_control {
    constexpr uint8_t BLACKLIGHT = 0x08;
    constexpr uint8_t NO_BLACKLIGHT = 0x00;
}

namespace lcd_bits {
    constexpr uint8_t En = 0x04;  // Enable bit
    constexpr uint8_t Rw = 0x02;  // Read/Write bit
    constexpr uint8_t Rs = 0x01;  // Register select bit
}

export class Display {
    I2C_HandleTypeDef& hi2c1;
    uint8_t addr;
    uint8_t display_function;
    uint8_t display_control;
    uint8_t display_mode;
    uint8_t num_lines;
    uint8_t rows;
    uint8_t back_light_val;

    void send_byte(uint8_t const value, uint8_t const mode) const {
        uint8_t const high_nib = value & 0xF0;
        uint8_t const low_nib = (value << 4) & 0xF0;
        write_4_bits(high_nib | mode);
        write_4_bits(low_nib | mode);
    }


    void write_4_bits(uint8_t const value) const {
        expander_write(value | lcd_bits::En);
        HAL_Delay_us(10);
        expander_write(value & ~lcd_bits::En);
        HAL_Delay_us(10);
    }


    void expander_write(uint8_t const value) const {
        uint8_t data = value | back_light_val;
        HAL_StatusTypeDef stat =  HAL_I2C_Master_Transmit(&hi2c1, addr, &data, 1, 100);

        if (stat != HAL_OK) LoggerMCU::exception();
    }
public:
    Display(
        I2C_HandleTypeDef& _hi2c1,
        uint8_t addr,
        uint8_t rows
    ) :
        hi2c1(_hi2c1),
        addr(addr << 1),
        display_function(
            lcd_function_set::LCD_4_BIT_MODE |
            lcd_function_set::LCD_1_LINE |
            lcd_function_set::LCD_5x8_DOTS),
        display_control(0),
        display_mode(0),
        num_lines(0),
        rows(rows),
        back_light_val(lcd_blacklight_control::BLACKLIGHT)
    {
        if (rows > 1) {
            display_function |= lcd_function_set::LCD_2_LINE;
        }
        num_lines = rows;

        if (rows == 1) {
            display_function |= lcd_function_set::LCD_5x10_DOTS;
        }

        HAL_Delay(50);

        expander_write(back_light_val);
        HAL_Delay(1000);

        write_4_bits(0x03 << 4);
        HAL_Delay(5);

        write_4_bits(0x03 << 4);
        HAL_Delay(5);

        write_4_bits(0x03 << 4);
        HAL_Delay(1);

        write_4_bits(0x02 << 4);

        send_command(lcd_commands::FUNCTION_SET | display_function);

        display_control =
            lcd_on_of_control_flags::DISPLAY_ON |
            lcd_on_of_control_flags::CURSOR_OFF |
            lcd_on_of_control_flags::BLINK_OFF;

        set_display();

        clear();

        display_mode =
            lcd_entry_mode_flags::ENTRY_LEFT |
            lcd_entry_mode_flags::ENTRY_SHIFT_DECREMENT;
        send_command(lcd_commands::ENTRY_MODE_SET | display_mode);

        return_home();
    }

    void clear() {
        send_command(lcd_commands::CLEAR_DISPLAY);
        HAL_Delay(2);
    }

    void return_home() {
        send_command(lcd_commands::RETURN_HOME);
        HAL_Delay(2);
    }

    void set_cursor(uint8_t const col, uint8_t row) {
        constexpr int row_offsets[] = {0x00, 0x40, 0x14, 0x54};
        if (row >= num_lines) {
            row = 0;
        }
        send_command(lcd_commands::SET_DDRAM_ADDR | (col + row_offsets[row]));
    }

    void set_no_display() {
        display_control &= ~lcd_on_of_control_flags::DISPLAY_ON;
        send_command(lcd_commands::DISPLAY_CONTROL | display_control);
    }

    void set_display() {
        display_control |= lcd_on_of_control_flags::DISPLAY_ON;
        send_command(lcd_commands::DISPLAY_CONTROL | display_control);
    }

    void set_no_cursor() {
        display_control &= ~lcd_on_of_control_flags::CURSOR_ON;
        send_command(lcd_commands::DISPLAY_CONTROL | display_control);
    }

    void set_cursor() {
        display_control |= lcd_on_of_control_flags::CURSOR_ON;
        send_command(lcd_commands::DISPLAY_CONTROL | display_control);
    }

    void set_no_blink() {
        display_control &= ~lcd_on_of_control_flags::BLINK_ON;
        send_command(lcd_commands::DISPLAY_CONTROL | display_control);
    }

    void set_blink() {
        display_control |= lcd_on_of_control_flags::BLINK_ON;
        send_command(lcd_commands::DISPLAY_CONTROL | display_control);
    }

    void scroll_left() {
        send_command(
            lcd_commands::CURSOR_SHIFT |
            lcd_cursor_shift::DISPLAY_MOVE |
            lcd_cursor_shift::MOVE_LEFT
        );
    }

    void scroll_right() {
        send_command(
            lcd_commands::CURSOR_SHIFT |
            lcd_cursor_shift::DISPLAY_MOVE |
            lcd_cursor_shift::MOVE_RIGHT
        );
    }

    void left_to_right() {
        display_mode |= lcd_entry_mode_flags::ENTRY_LEFT;
        send_command(lcd_commands::ENTRY_MODE_SET | display_mode);
    }

    void right_to_left() {
        display_mode &= ~lcd_entry_mode_flags::ENTRY_LEFT;
        send_command(lcd_commands::ENTRY_MODE_SET | display_mode);
    }

    void auto_scroll() {
        display_mode |= lcd_entry_mode_flags::ENTRY_SHIFT_INCREMENT;
        send_command(lcd_commands::ENTRY_MODE_SET | display_mode);
    }

    void no_auto_scroll() {
        display_mode &= ~lcd_entry_mode_flags::ENTRY_SHIFT_INCREMENT;
        send_command(lcd_commands::ENTRY_MODE_SET | display_mode);
    }

    void create_char(uint8_t location, uint8_t char_map[]) {
        location &= 0x7;
        send_command(lcd_commands::SET_CGRAM_ADDR | (location << 3));
        for (int i = 0; i < 8; i++) {
            write(char_map[i]);
        }
    }

    void no_blacklight() {
        back_light_val = lcd_blacklight_control::NO_BLACKLIGHT;
        expander_write(0);
    }

    void blacklight() {
        back_light_val = lcd_blacklight_control::BLACKLIGHT;
        expander_write(0);
    }

    void send_command(uint8_t const value) {
        send_byte(value, 0);
    }

    void write(uint8_t const value) {
        send_byte(value, lcd_bits::Rs);
        HAL_Delay(1);
    }

    void print(std::span<const char> const str) {
        for (char const ch : str) {
            if (ch == '\0') return;
            write(ch);
        }
    }
};