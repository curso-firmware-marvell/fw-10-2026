#ifndef HAL_RING_H
#define HAL_RING_H

#include <stdint.h>

/** Capacidad del ring en este laboratorio (las actividades de clase usan 64). */
#define RING_BUF_SIZE 32

typedef struct {
    uint8_t  data[RING_BUF_SIZE];
    uint16_t head;
    uint16_t tail;
    uint16_t count;
} ring_buf_t;

void ring_init(ring_buf_t *rb);
int  ring_push(ring_buf_t *rb, uint8_t byte);
int  ring_pop(ring_buf_t *rb, uint8_t *byte);

static inline int ring_is_empty(const ring_buf_t *rb) {
    return rb->count == 0;
}

static inline int ring_is_full(const ring_buf_t *rb) {
    return rb->count >= RING_BUF_SIZE;
}

#endif /* HAL_RING_H */
