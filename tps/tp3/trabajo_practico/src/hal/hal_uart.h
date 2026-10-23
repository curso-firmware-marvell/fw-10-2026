#ifndef HAL_UART_H
#define HAL_UART_H

#include <stdint.h>
#include "mathlib.h"

/** Reloj del periférico UART en este laboratorio (distinto de `../actividades_de_clase/`). */
#define UART_PERIPH_CLK_HZ  16000000U

typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
} uart_regs_t;

void uart_init(uart_regs_t *uart, uint32_t baudrate);
void uart_send_byte(uart_regs_t *uart, uint8_t byte);
void uart_send(uart_regs_t *uart, const uint8_t *data, uint32_t len);

#endif /* HAL_UART_H */
