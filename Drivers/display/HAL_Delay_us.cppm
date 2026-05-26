module;

#include "main.h"

export module HAL_Delay_us;

export void HAL_Delay_us(uint32_t const us) {
    uint32_t const ticks_per_us = SystemCoreClock / 1'000'000;

    uint32_t const total_ticks = ticks_per_us * us;

    uint32_t start_tick = SysTick->VAL;
    uint32_t elapsed_ticks = 0;
    uint32_t current_tick = 0;

    while (elapsed_ticks < total_ticks) {
        current_tick = SysTick->VAL;

        if (current_tick < start_tick) {
            elapsed_ticks += (start_tick - current_tick);
        } else if (current_tick > start_tick) {
            elapsed_ticks += (start_tick + (SysTick->LOAD - current_tick));
        }

        start_tick = current_tick;
    }
}