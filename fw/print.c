// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.

#include "includes.h"

// REG_UART_COMMAND : 
//					   0x00 -> reset
//					   0x01 -> data ready
//					   0x02 -> flush peripheral buffer
void print_chr(char ch)
{
	REG_UART_COMMAND = 0;
    REG_UART_DATA = ch;
	REG_UART_COMMAND = 1;
}

void print_str(const char *p)
{
    while (*p != 0){
		REG_UART_COMMAND = 0;
        REG_UART_DATA = *(p++);
		REG_UART_COMMAND = 1;
	}
	print_flush_buffer();
}

void print_dec(unsigned int val)
{
    char buffer[10];
    char *p = buffer;
    while (val || p == buffer) {
        *(p++) = val % 10;
        val = val / 10;
    }
    while (p != buffer) {
		REG_UART_COMMAND = 0;
        REG_UART_DATA = '0' + *(--p);
		REG_UART_COMMAND = 1;
    }
	print_flush_buffer();
}

void print_hex(unsigned int val, int digits)
{
    for (int i = (4*digits)-4; i >= 0; i -= 4){
		REG_UART_COMMAND = 0;
        REG_UART_DATA = "0123456789ABCDEF"[(val >> i) % 16];
		REG_UART_COMMAND = 1;
	}
	print_flush_buffer();
}

void print_flush_buffer()
{
	REG_UART_COMMAND = 2;
	REG_UART_COMMAND = 0;
}