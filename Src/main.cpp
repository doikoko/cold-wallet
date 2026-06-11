#include <span>

#include "main.h"
#include "stm32f4xx_hal_gpio.h"

import display;
import logger_mcu;
import uart;
import result;

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
}