#include "main.h"
#include "stm32f4xx_hal_gpio.h"

import display;
import HAL_Delay_us;

constexpr inline void led_enable() {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

extern "C" void main_cpp() {
    led_enable();


}
