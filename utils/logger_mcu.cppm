module;

#include <memory>
#include <span>
#include <ranges>

#include "main.h"

export module logger_mcu;

export class LoggerMCU {
    static constexpr inline void led_enable() {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    }

    static constexpr inline void led_disable() {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    }
public:
    static void exception() {
        led_enable();

        while (true) {
            asm("wfi");
        }
    }
};