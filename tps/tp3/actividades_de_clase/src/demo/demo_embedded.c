/**
 * @file    demo_embedded.c
 * @brief   Demonstration of embedded HAL patterns and inline assembly.
 *
 * Exercises the UART driver, ring buffer, SysTick ISR simulation,
 * external assembly functions (@c asm_example.s), and
 * architecture-specific inline assembly intrinsics (CLZ, CTZ, BSWAP,
 * rotate, RDTSC, memory barrier, NOP, PAUSE).
 */

#include "demo.h"
#include "mathlib.h"
#include "hal_uart.h"
#include "hal_ring.h"
#include "hal_system.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/** @brief Simulated UART register block (file-scope). */
static uart_regs_t sim_uart;

/** @brief Simulated RX ring buffer (file-scope). */
static ring_buf_t  rx_ring;

void demo_embedded(void) {
    board_early_init();

    /* UART simulation */
    uart_init(&sim_uart, 115200);
    printf("[uart] BRR=%u, CR1=0x%08X\n",
           (unsigned)sim_uart.BRR, (unsigned)sim_uart.CR1);

    const char *msg = "OK\r\n";
    for (const char *p = msg; *p; p++)
        uart_send_byte(&sim_uart, (uint8_t)*p);
    printf("[uart] Sent %zu bytes, SR=0x%08X\n", strlen(msg), (unsigned)sim_uart.SR);

    /* Ring buffer */
    ring_init(&rx_ring);
    for (uint8_t i = 0; i < 10; i++)
        ring_push(&rx_ring, 'A' + i);
    printf("[ring] Pushed 10 bytes, count=%u\n", (unsigned)rx_ring.count);

    uint8_t ch;
    printf("[ring] Popped : ");
    while (ring_pop(&rx_ring, &ch) == 0)
        printf("%c", ch);
    printf("\n");

    /* ISR simulation */
    for (int i = 0; i < 5; i++)
        SysTick_Handler();
    printf("[isr]  isr_counter=%u, flags=0x%08X\n",
           (unsigned)system_get_isr_count(), (unsigned)system_flags);

    /* External assembly functions (from asm_example.s) */
    printf("[asm]  asm_add(10, 20)   = %d\n", asm_add(10, 20));
    printf("[asm]  asm_mul(6, 7)     = %d\n", asm_mul(6, 7));
    printf("[asm]  asm_negate(42)    = %d\n", asm_negate(42));

    /* Inline assembly — bit scanning */
    printf("[iasm] clz(0x00F0)       = %u\n", (unsigned)asm_clz(0x00F0));
    printf("[iasm] ctz(0x00F0)       = %u\n", (unsigned)asm_ctz(0x00F0));
    printf("[iasm] clz(1)            = %u\n", (unsigned)asm_clz(1));
    printf("[iasm] ctz(0x80000000)   = %u\n", (unsigned)asm_ctz(0x80000000U));

    /* Inline assembly — endianness conversion */
    printf("[iasm] bswap32(0xDEADBEEF) = 0x%08X\n",
           (unsigned)asm_bswap32(0xDEADBEEF));
    printf("[iasm] bswap16(0xABCD)     = 0x%04X\n",
           (unsigned)asm_bswap16(0xABCD));

    /* Inline assembly — bit rotation */
    printf("[iasm] rol32(0x01, 4)    = 0x%08X\n",
           (unsigned)asm_rol32(0x01, 4));
    printf("[iasm] ror32(0x10, 4)    = 0x%08X\n",
           (unsigned)asm_ror32(0x10, 4));

    /* Inline assembly — CPU timestamp counter */
    uint64_t t0 = asm_rdtsc();
    asm_nop();
    asm_nop();
    asm_nop();
    asm_pause();
    uint64_t t1 = asm_rdtsc();
    printf("[iasm] rdtsc delta (3xNOP+PAUSE) = %lu cycles\n",
           (unsigned long)(t1 - t0));

    /* Inline assembly — memory barrier */
    volatile uint32_t sensor_val = 0x42;
    asm_memory_barrier();
    printf("[iasm] memory_barrier OK, sensor=0x%X\n", sensor_val);
}
