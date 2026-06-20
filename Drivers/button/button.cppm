module;

#include "main.h"

export module button;

export class Button {
    GPIO_TypeDef* gpio;
    uint16_t letter;
public:
    Button(GPIO_TypeDef* const gpio, uint16_t const letter) :
        gpio(gpio),
        letter(letter){}

    [[nodiscard]] constexpr bool is_pressed() const {
        return HAL_GPIO_ReadPin(gpio, letter) == GPIO_PIN_RESET;
    }
};