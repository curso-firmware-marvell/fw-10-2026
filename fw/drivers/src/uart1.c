/* Driver de la UART1 serie.
 *
 * Punto 3.b del TP5: completar los cuerpos marcados con TODO usando los registros de
 *         datasheets/UART1_DATASHEET.md (REG_UART1_*, definidos en config.h).
 * TP6: completar uart1_set_rts().
 *
 * Hoy los cuerpos no hacen nada (solo para que el proyecto compile): make
 * test_uart_serial tiene que fallar hasta que los implementes.
 */
#include "uart1.h"

void uart1_init(uint32_t baud_div)
{
    /* TODO 3.b: programar BAUD_DIV y limpiar los flags de error. */
    (void)baud_div;
}

uint32_t uart1_rx_level(void)
{
    /* TODO 3.b: cantidad de bytes en el FIFO RX (ver STATUS). */
    return 0;
}

int uart1_getc(uint8_t *b)
{
    /* TODO 3.b: leer RX_DATA (leer saca un byte del FIFO) y devolver 1 si el
     * bit VALID estaba en 1. Si no, devolver 0 sin tocar *b. */
    (void)b;
    return 0;
}

void uart1_putc(uint8_t b)
{
    /* TODO 3.b: esperar a que el FIFO TX tenga lugar y escribir TX_DATA. */
    (void)b;
}

void uart1_write(const uint8_t *buf, uint32_t n)
{
    /* TODO 3.b */
    (void)buf;
    (void)n;
}

int uart1_tx_idle(void)
{
    /* TODO 3.b */
    return 0;
}

uint32_t uart1_errors(void)
{
    /* TODO 3.b: UART1_ERR_FRAME / UART1_ERR_OVERRUN segun el STATUS. */
    return 0;
}

void uart1_clear_errors(void)
{
    /* TODO 3.b: pulsar CLR_ERR en CTRL sin cambiar el estado de RTS. */
}

/* ---- TP6 ---- */

void uart1_set_rts(int ready)
{
    /* TODO TP6 */
    (void)ready;
}