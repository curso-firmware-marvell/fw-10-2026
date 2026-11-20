#ifndef CLOCK_H
#define CLOCK_H

/* Tiempo en clocks (provisto por la catedra, no modificar).
 *
 * La plataforma no tiene un timer programable. Lo que hay es el contador de ciclos del propio
 * core (el PicoRV32 tiene los contadores habilitados): sube de a uno por cada clock de la
 * simulacion. Son las mismas unidades que usan los patrones de test ("wait 150000",
 * "timeout 600000") y el tiempo de un byte de la UART1 (10 * BAUD_DIV = 320 clocks).
 *
 * Uso tipico, para medir cuanto paso desde un evento:
 *
 *     uint32_t t0 = clock_cycles();               // anotar la hora
 *     ...
 *     if (clock_elapsed(t0) > 40000) { ... }      // ya pasaron mas de 40000 clocks?
 */
#include <stdint.h>

/* Contador de ciclos del core (32 bits). */
static inline uint32_t clock_cycles(void)
{
    uint32_t c;
    __asm__ volatile ("rdcycle %0" : "=r"(c));
    return c;
}

/* Clocks que pasaron desde 'since' (un valor que devolvio clock_cycles()). La resta sin signo
 * da bien aunque el contador de 32 bits haya dado la vuelta en el medio. */
static inline uint32_t clock_elapsed(uint32_t since)
{
    return clock_cycles() - since;
}

#endif /* CLOCK_H */
