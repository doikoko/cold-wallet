module;

#include <memory>
#include <span>
#include <ranges>

#include "main.h"

export module logger_mcu;

export class LED {
public:
    static constexpr void enable() {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    }

    static constexpr void disable() {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    }
};
export class LoggerMCU : LED {
public:
    static void exception() {
        enable();

        while (true) {
            asm("wfi");
        }
    }
};