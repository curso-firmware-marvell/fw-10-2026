#ifndef PRINT_H
#define PRINT_H

/*
 * Salida de consola.
 * print.c implementa las funciones de formato sobre print_chr();
 * cada plataforma (src/platform/<plataforma>/) implementa print_chr() y print_flush_buffer().
 */

void print_chr(char ch);
void print_str(const char *p);
void print_dec(unsigned int val);
void print_hex(unsigned int val, int digits);
void print_flush_buffer(void);

#endif /* PRINT_H */
