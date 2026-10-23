#ifndef HAL_TIMER_H
#define HAL_TIMER_H

#include <stdint.h>

/**
 * Temporizador software simple (no existe en las actividades de clase — implementación propia).
 * Cada llamada a timer_tick() suma 1 ms al contador.
 */
void     timer_init(void);
void     timer_tick(void);
uint32_t timer_get_ms(void);

#endif /* HAL_TIMER_H */
