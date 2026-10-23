#include <stdio.h>
#include "print.h"

void print_chr(char ch)
{
    putchar(ch);
}

void print_flush_buffer(void)
{
    fflush(stdout);
}
