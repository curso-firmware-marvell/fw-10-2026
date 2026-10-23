/**
 * @file    hal_uart.h
 * @brief   Simulated UART peripheral driver using memory-mapped register struct.
 *
 * Models the register layout of a typical MCU UART (CR1, CR2, SR, DR, BRR).
 * All state is stored in a caller-provided @ref uart_regs_t, so multiple
 * UART instances can coexist without static globals.
 */

#ifndef HAL_UART_H
#define HAL_UART_H

#include <stdint.h>
#include "mathlib.h"

/**
 * @brief   Simulated UART register block.
 *
 * Fields are marked @c volatile to mirror real hardware where the
 * peripheral may change status bits asynchronously.
 */
typedef struct {
    volatile uint32_t CR1;  /**< Control register 1 (UE, TE, RE bits). */
    volatile uint32_t CR2;  /**< Control register 2 (reserved in this sim). */
    volatile uint32_t SR;   /**< Status register (TC bit, etc.). */
    volatile uint32_t DR;   /**< Data register (last byte written). */
    volatile uint32_t BRR;  /**< Baud-rate register (clock / baudrate). */
} uart_regs_t;

/**
 * @brief   Initialise a UART instance.
 *
 * Configures the baud-rate register and enables USART, TX, and RX in CR1.
 *
 * @param   uart        Pointer to the register block to initialise.
 * @param   baudrate    Desired baud rate (assumes 8 MHz peripheral clock).
 */
void uart_init(uart_regs_t *uart, uint32_t baudrate);

/**
 * @brief   Transmit a single byte.
 * @param   uart    Pointer to an initialised UART register block.
 * @param   byte    Byte to send.
 */
void uart_send_byte(uart_regs_t *uart, uint8_t byte);

/**
 * @brief   Transmit a buffer of bytes.
 * @param   uart    Pointer to an initialised UART register block.
 * @param   data    Pointer to the data buffer.
 * @param   len     Number of bytes to send.
 */
void uart_send(uart_regs_t *uart, const uint8_t *data, uint32_t len);

#endif /* HAL_UART_H */
