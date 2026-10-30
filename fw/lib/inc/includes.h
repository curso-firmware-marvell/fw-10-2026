//#define REG_FW_STATUS (*(volatile unsigned int*)0x000040000)
//#define REG_FW_REPORT (*(volatile unsigned int*)0x000040004)
//#define REG_UART      (*(volatile unsigned int*)0x000040008)
//#define REG_AUX_0     (*(volatile unsigned int*)0x00004000c)

#include "config.h"

// print.c
void print_chr(char ch);
void print_str(const char *p);
void print_dec(unsigned int val);
void print_hex(unsigned int val, int digits);
void print_flush_buffer(void);


// // RX
// int uart_rx_available(void);
// unsigned int uart_rx_read(void);
// unsigned int uart_rx_count(void);
// void uart_rx_flush(void);