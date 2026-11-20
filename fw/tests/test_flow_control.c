/* Hito 1: recibir una rafaga continua de bytes sin perder ninguno.
 *
 * Protocolo de la prueba:
 *   el host manda  len_lo len_hi  y despues 'len' bytes seguidos, sin pausas
 *   la app procesa cada byte con procesar_lento() (slow_consumer.h)
 *   la app responde  crc8  len_lo  len_hi
 *
 * El consumidor es mas lento que la linea: mientras procesar_lento() corre no se mira la
 * UART y el FIFO RX (UART1_FIFO_DEPTH bytes) se va llenando. El host respeta la linea RTS
 * (ver datasheets/UART1_DATASHEET.md, "Flow control"): si la bajan, deja de mandar despues del
 * byte en curso (mas 'skid' bytes que ya tenia en camino), y espera hasta que la suban.
 *
 * Ya esta escrito: leer los bytes, procesarlos y responder. Falta el control de flujo, en
 * flow_control() (mas abajo): BAJAR RTS cuando el FIFO esta cerca de llenarse y VOLVER A
 * SUBIRLO cuando se vacio. Tambien hay que completar uart1_set_rts() en fw/drivers/src/uart1.c.
 *
 * Sin la mitad que baja RTS se pierden bytes (overrun); sin la que lo sube el host se queda
 * esperando para siempre. El check falla en los dos casos.
 */
#include "uart1.h"
#include "slow_consumer.h"

static int rts_on;      /* 1 = RTS en alto (el host puede mandar) */

static void flow_control(void)
{
    uint32_t level = uart1_rx_level();

    if (rts_on) {
        /* TODO hito 1: si el FIFO esta cerca de llenarse, bajar RTS (uart1_set_rts(0)) y
         * anotarlo en rts_on.
         *
         * Pensar con cuanto margen: despues de bajar RTS todavia pueden llegar hasta 3
         * bytes (el que estaba en curso y 'skid' = 2), y mientras procesar_lento() corre
         * entran hasta 3 mas. El FIFO tiene UART1_FIFO_DEPTH (16) bytes. */
        (void)level;
    } else {
        /* TODO hito 1: si el FIFO ya se vacio lo suficiente, volver a subir RTS
         * (uart1_set_rts(1)) y anotarlo en rts_on.
         *
         * Pensar: el nivel para volver a subir tiene que ser menor que el de bajar, si no
         * RTS oscila con cada byte. */
        (void)level;
    }
}

static uint8_t rx_byte(void)
{
    uint8_t b;
    for (;;) {
        flow_control();
        if (uart1_getc(&b))
            return b;
    }
}

void main(void)
{
    REG_FW_STATUS = 0;
    uart1_init(UART1_SIM_BAUD_DIV);
    uart1_set_rts(1);
    rts_on = 1;

    for (;;) {
        uint32_t lo  = rx_byte();
        uint32_t hi  = rx_byte();
        uint32_t len = lo | (hi << 8);

        procesar_reset();
        for (uint32_t i = 0; i < len; i++)
            procesar_lento(rx_byte());

        uart1_putc(procesar_crc());
        uart1_putc((uint8_t)lo);
        uart1_putc((uint8_t)hi);
    }
}
