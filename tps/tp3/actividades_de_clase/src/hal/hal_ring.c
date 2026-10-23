/**
 * @file    hal_ring.c
 * @brief   Implementation of the static circular (ring) buffer.
 *
 * @see     hal_ring.h for the public API.
 */

#include "hal_ring.h"
#include <string.h>

void ring_init(ring_buf_t *rb) {
    memset(rb->data, 0, sizeof(rb->data));
    rb->head  = 0;
    rb->tail  = 0;
    rb->count = 0;
}

int ring_push(ring_buf_t *rb, uint8_t byte) {
    if (rb->count >= RING_BUF_SIZE)
        return -1;
    rb->data[rb->head] = byte;
    rb->head = (rb->head + 1) % RING_BUF_SIZE;
    rb->count++;
    return 0;
}

int ring_pop(ring_buf_t *rb, uint8_t *byte) {
    if (rb->count == 0)
        return -1;
    *byte = rb->data[rb->tail];
    rb->tail = (rb->tail + 1) % RING_BUF_SIZE;
    rb->count--;
    return 0;
}
