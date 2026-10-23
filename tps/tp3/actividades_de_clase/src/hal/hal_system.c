/**
 * @file    hal_system.c
 * @brief   Implementation of board-level system services: weak BSP hook,
 *          SysTick ISR simulation, and global system state.
 *
 * @see     hal_system.h for the public API.
 */

#include "hal_system.h"
#include "mathlib.h"
#include <stdio.h>

/** @brief Bit-field of event flags (shared with ISR). */
volatile uint32_t system_flags = 0;

/** @brief Uninitialized global in .bss for ELF section analysis. */
int uninitialized_global;

/** @brief Cumulative count of SysTick interrupts. */
static volatile uint32_t isr_counter = 0;

void board_early_init(void) __attribute__((weak));
void board_early_init(void) {
    printf("[weak] Default board_early_init (no custom BSP)\n");
}

void SysTick_Handler(void) {
    isr_counter++;
    system_flags = bit_set(system_flags, 0);
}

uint32_t system_get_isr_count(void) {
    return isr_counter;
}
