/*
 * test_data_bss.c — TP4 (Bloque 4): test de aceptación del "De 0 a main" del FW
 * (copia de .data desde la imagen a la RAM y .bss en cero). Se usa como FW
 * "del TP anterior" en lugar de test_fw_status.c. Con la solución de referencia:
 *
 *   make sim APP_SRC="print.c test_data_bss.c"                -> PASS (arranque del TP4)
 *   make sim APP_SRC="print.c test_data_bss.c" STANDALONE=1   -> FAIL (fw/startup.S:
 *        nadie copia .data, así que las globales inicializadas leen 0 en vez de su valor)
 *
 * Ojo: que .bss lea 0 prueba poco en simulación, porque la RAM del modelo ya
 * arranca en 0. Lo que sí se ve es la copia de .data.
 */
#include "includes.h"

volatile unsigned int tp4_data_word     = 0x1234ABCD;   /* 4 B  -> .sdata (small data de RISC-V) */
volatile unsigned int tp4_data_array[4] = {1, 2, 3, 4}; /* 16 B -> .data                         */
volatile unsigned int tp4_bss_word;                     /* 4 B  -> .sbss                         */
volatile unsigned int tp4_bss_array[16];                /* 64 B -> .bss                          */

/* "<msg>0xXXXXXXXX" en una sola transmisión de UART0 (una línea en el testbench) */
static void log_hex(const char *msg, unsigned int val)
{
    char line[64];
    unsigned int n = 0;

    while (*msg != '\0' && n < sizeof(line) - 11u)
        line[n++] = *msg++;
    line[n++] = '0';
    line[n++] = 'x';
    for (int shift = 28; shift >= 0; shift -= 4)
        line[n++] = "0123456789ABCDEF"[(val >> shift) & 0xFu];
    line[n] = '\0';
    print_str(line);
}

void main(void)
{
    unsigned int errors = 0;

    REG_FW_STATUS = 0x0;
    print_str("[FW] test_data_bss: chequeo de .data/.bss");
    log_hex("[FW]   &tp4_data_word  = ", (unsigned int)&tp4_data_word);   /* tiene que caer en RAM */
    log_hex("[FW]    tp4_data_word  = ", tp4_data_word);
    log_hex("[FW]   &tp4_bss_array  = ", (unsigned int)&tp4_bss_array);

    if (tp4_data_word != 0x1234ABCD) {
        print_str("[FW] FAIL: tp4_data_word (.sdata) no tiene su valor inicial");
        errors++;
    }
    for (unsigned int i = 0; i < 4; i++) {
        if (tp4_data_array[i] != i + 1) {
            print_str("[FW] FAIL: tp4_data_array (.data) no tiene su valor inicial");
            errors++;
            break;
        }
    }
    if (tp4_bss_word != 0) {
        print_str("[FW] FAIL: tp4_bss_word (.sbss) no esta en cero");
        errors++;
    }
    for (unsigned int i = 0; i < 16; i++) {
        if (tp4_bss_array[i] != 0) {
            print_str("[FW] FAIL: tp4_bss_array (.bss) no esta en cero");
            errors++;
            break;
        }
    }

    if (errors == 0) {
        print_str("[FW] test_data_bss: PASS");
        REG_FW_STATUS = 0xFF;
    } else {
        print_str("[FW] test_data_bss: FAIL");
        REG_FW_STATUS = 0xBAD;
        __asm__ volatile ("ebreak");    /* trap: con 0xBAD solo, la simulación no termina */
    }
}
