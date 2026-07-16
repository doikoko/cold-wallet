module;

#include "main.h"

export module button;

export class Button {
    [[nodiscard]] constexpr bool is_pressed() const {
        return HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET;
    }
public:
    void wait_pressed() const {
        while (!is_pressed())
            HAL_Delay(1);
    }
};