#include "print.h"

void print_str(const char *p)
{
    while (*p != '\0')
        print_chr(*p++);
}

void print_dec(unsigned int val)
{
    char digits[10];
    unsigned int n = 0;

    do {
        digits[n++] = (char)('0' + val % 10);
        val /= 10;
    } while (val != 0);

    while (n > 0)
        print_chr(digits[--n]);
}

void print_hex(unsigned int val, int digits)
{
    if (digits > 8)
        digits = 8;
    for (int i = (4 * digits) - 4; i >= 0; i -= 4)
        print_chr("0123456789ABCDEF"[(val >> i) & 0xF]);
}
