#ifndef SLOW_CONSUMER_H
#define SLOW_CONSUMER_H

/* Consumidor lento del hito 1 (provisto por la catedra, no modificar).
 *
 * procesar_lento() acumula un CRC-8 (poly 0x07, init 0xFF, el de str_crc8()) de los datos y tarda
 * varios tiempos de byte de UART1 en volver (un byte tarda 10 * BAUD_DIV clocks en
 * llegar). Mientras esta adentro el FW no puede mirar la UART, asi que el host
 * sigue mandando: ahi es donde se llena el FIFO RX si nadie frena al host.
 */
#include <stdint.h>

#define SLOW_ITERS   40

static uint8_t g_crc;

static inline void procesar_reset(void)
{
    g_crc = 0xFF;
}

static inline uint8_t procesar_crc(void)
{
    return g_crc;
}

static void procesar_lento(uint8_t b)
{
    uint8_t crc = g_crc ^ b;
    for (int i = 0; i < 8; i++)
        crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x07) : (uint8_t)(crc << 1);
    g_crc = crc;

    for (volatile int i = 0; i < SLOW_ITERS; i++)
        ;
}

#endif /* SLOW_CONSUMER_H */
