/**
 * @file    hal_system.h
 * @brief   Board-level system services: GPIO definitions, ISR handler,
 *          weak BSP hook, and external assembly function declarations.
 *
 * Provides the "glue" between the application and the (simulated)
 * hardware: interrupt counters, system flags, and assembly-level
 * functions defined in @c asm_example.s.
 */

#ifndef HAL_SYSTEM_H
#define HAL_SYSTEM_H

#include <stdint.h>

/** @brief Base address of the (simulated) GPIO port A. */
#define GPIO_BASE       0x40020000U

/** @brief GPIO mode register (simulated, memory-mapped). */
#define GPIO_MODER      (*(volatile uint32_t *)(uintptr_t)(GPIO_BASE + 0x00))

/** @brief GPIO output data register (simulated, memory-mapped). */
#define GPIO_ODR        (*(volatile uint32_t *)(uintptr_t)(GPIO_BASE + 0x14))

/**
 * @brief   Bit-field of system event flags, shared between ISR and main loop.
 *
 * Bit 0 is set by SysTick_Handler() on every tick.
 */
extern volatile uint32_t system_flags;

/**
 * @brief   Uninitialized global placed in .bss for binutils analysis.
 */
extern int uninitialized_global;

/**
 * @brief   Weak BSP early-init hook — override in board-specific code.
 *
 * Declared with @c __attribute__((weak)); the default implementation
 * simply prints a diagnostic message.
 */
void board_early_init(void);

/**
 * @brief   Simulated SysTick interrupt handler.
 *
 * Increments the internal ISR counter and sets bit 0 of @ref system_flags.
 */
void SysTick_Handler(void);

/**
 * @brief   Read the cumulative SysTick ISR invocation count.
 * @return  Number of times SysTick_Handler() has been called.
 */
uint32_t system_get_isr_count(void);

/**
 * @brief   Add two integers (implemented in asm_example.s).
 * @param   a   First operand.
 * @param   b   Second operand.
 * @return  a + b.
 */
extern int asm_add(int a, int b);

/**
 * @brief   Multiply two integers (implemented in asm_example.s).
 * @param   a   First operand.
 * @param   b   Second operand.
 * @return  a * b.
 */
extern int asm_mul(int a, int b);

/**
 * @brief   Negate an integer (implemented in asm_example.s).
 * @param   a   Input value.
 * @return  -a.
 */
extern int asm_negate(int a);

#endif /* HAL_SYSTEM_H */
