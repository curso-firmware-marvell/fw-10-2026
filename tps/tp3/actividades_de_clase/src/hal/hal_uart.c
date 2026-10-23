/**
 * @file    hal_uart.c
 * @brief   Implementation of the simulated UART peripheral driver.
 *
 * @see     hal_uart.h for the public API.
 */

#include "hal_uart.h"

void uart_init(uart_regs_t *uart, uint32_t baudrate) {
    uart->BRR = 8000000U / baudrate;
    uart->CR1 = bit_set(0, 13);            /* UE: USART enable  */
    uart->CR1 = bit_set(uart->CR1, 3);     /* TE: TX enable     */
    uart->CR1 = bit_set(uart->CR1, 2);     /* RE: RX enable     */
    uart->SR  = 0;
    uart->DR  = 0;
}

void uart_send_byte(uart_regs_t *uart, uint8_t byte) {
    uart->DR = byte;
    uart->SR = bit_set(uart->SR, 6);       /* TC: transfer complete */
}

void uart_send(uart_regs_t *uart, const uint8_t *data, uint32_t len) {
    for (uint32_t i = 0; i < len; i++)
        uart_send_byte(uart, data[i]);
}
