#ifndef UART1_H
#define UART1_H

#include <stdint.h>
#include "includes.h"      /* config.h: REG_UART1_*, UART1_FIFO_DEPTH, UART1_SIM_BAUD_DIV */

/* ------------------------------------------------------------------
 * Driver de la UART1 serie (ver datasheets/UART1_DATASHEET.md).
 * Los cuerpos de las funciones estan en uart1.c: completarlos.
 * ------------------------------------------------------------------ */

/* Bits de REG_UART1_STATUS */
#define UART1_ST_RX_NOT_EMPTY   (1u << 0)
#define UART1_ST_RX_FULL        (1u << 1)
#define UART1_ST_OVERRUN        (1u << 2)   /* sticky */
#define UART1_ST_FRAME_ERR      (1u << 3)   /* sticky */
#define UART1_ST_TX_FULL        (1u << 4)
#define UART1_ST_TX_EMPTY       (1u << 5)
#define UART1_ST_TX_IDLE        (1u << 6)   /* FIFO vacio y ultimo byte ya enviado */
#define UART1_ST_CTS            (1u << 7)
#define UART1_ST_RX_LEVEL(s)    (((s) >> 8)  & 0xFFu)
#define UART1_ST_TX_LEVEL(s)    (((s) >> 16) & 0xFFu)

/* Bits de REG_UART1_CTRL */
#define UART1_CTRL_RTS          (1u << 0)   /* 1 = estoy listo para recibir */
#define UART1_CTRL_CLR_ERR      (1u << 1)   /* escribir 1 limpia OVERRUN y FRAME_ERR */

/* Bit de REG_UART1_RX_DATA */
#define UART1_RX_VALID          (1u << 8)

/* Flags que devuelve uart1_errors() */
#define UART1_ERR_FRAME         (1u << 0)
#define UART1_ERR_OVERRUN       (1u << 1)

/* ---------------- 3.b (TP5) ---------------- */

/* Programa BAUD_DIV (clocks por bit) y limpia los flags de error. */
void     uart1_init(uint32_t baud_div);

/* Cantidad de bytes esperando en el FIFO RX. */
uint32_t uart1_rx_level(void);

/* Lee un byte del FIFO RX sin bloquear. Devuelve 1 y guarda el byte en *b si habia
 * uno; devuelve 0 si el FIFO estaba vacio (y no toca *b). */
int      uart1_getc(uint8_t *b);

/* Manda un byte. Si el FIFO TX esta lleno, espera a que haya lugar. */
void     uart1_putc(uint8_t b);

/* Manda n bytes (bloqueante). */
void     uart1_write(const uint8_t *buf, uint32_t n);

/* 1 si el FIFO TX esta vacio y el ultimo byte ya salio por la linea. */
int      uart1_tx_idle(void);

/* UART1_ERR_FRAME | UART1_ERR_OVERRUN segun los flags sticky del STATUS. */
uint32_t uart1_errors(void);

/* Limpia los flags sticky. */
void     uart1_clear_errors(void);

/* ---------------- TP6: control de flujo ---------------- */

/* ready=1: RTS activo (el host puede mandar). ready=0: pide al host que pare. */
void     uart1_set_rts(int ready);

#endif /* UART1_H */