/* TP5 3.b: aplicacion de prueba del driver de UART1 (provista por la catedra).
 *
 * Protocolo de un byte de comando, sin control de flujo:
 *   al arrancar    -> manda 'K'
 *   'E' n d0..dn-1 -> devuelve los mismos n bytes (eco)
 *   'S' k          -> deja de leer UART1 durante un rato (provoca overrun), despues manda 'S'
 *   'R'            -> manda 'R' flags (bit0 = framing error, bit1 = overrun) y limpia los flags
 *   cualquier otro -> se ignora
 *
 * El host del test siempre espera RTS. Como el control de flujo es del TP6, esta app deja
 * RTS en alto escribiendo el registro directamente (no usa uart1_set_rts()). Lo repite antes de
 * cada lectura porque escribir REG_UART1_CTRL reemplaza los dos bits: un uart1_clear_errors()
 * que escriba solo CLR_ERR bajaria RTS.
 */
#include "uart1.h"

static void rts_high(void)
{
    REG_UART1_CTRL = UART1_CTRL_RTS;
}

static uint8_t get_byte(void)
{
    uint8_t b;
    rts_high();
    while (!uart1_getc(&b))
        ;
    return b;
}

void main(void)
{
    REG_FW_STATUS = 0;
    uart1_init(UART1_SIM_BAUD_DIV);
    rts_high();
    uart1_putc('K');

    for (;;) {
        uint8_t cmd = get_byte();

        if (cmd == 'E') {
            uint8_t n = get_byte();
            for (uint32_t i = 0; i < n; i++)
                uart1_putc(get_byte());
        }
        else if (cmd == 'S') {
            uint32_t k = get_byte();
            for (volatile uint32_t i = 0; i < k * 200; i++)
                ;
            uart1_putc('S');
        }
        else if (cmd == 'R') {
            uint32_t e = uart1_errors();
            uart1_clear_errors();
            uart1_putc('R');
            uart1_putc((uint8_t)e);
        }
    }
}