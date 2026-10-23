/**
 * @file    hal_ring.h
 * @brief   Static circular (ring) buffer — no heap allocation.
 *
 * Commonly used in embedded firmware to transfer data between an ISR
 * producer and a main-loop consumer without dynamic memory.
 */

#ifndef HAL_RING_H
#define HAL_RING_H

#include <stdint.h>

/** @brief Capacity of the ring buffer in bytes. */
#define RING_BUF_SIZE 64

/**
 * @brief   Statically-allocated circular byte buffer.
 */
typedef struct {
    uint8_t  data[RING_BUF_SIZE];   /**< Storage array.        */
    uint16_t head;                  /**< Write index.          */
    uint16_t tail;                  /**< Read index.           */
    uint16_t count;                 /**< Current occupancy.    */
} ring_buf_t;

/**
 * @brief   Initialise (or reset) a ring buffer to empty.
 * @param   rb  Buffer to initialise.
 */
void ring_init(ring_buf_t *rb);

/**
 * @brief   Push a byte into the ring buffer.
 * @param   rb      Target buffer.
 * @param   byte    Byte to enqueue.
 * @return  0 on success, -1 if the buffer is full.
 */
int  ring_push(ring_buf_t *rb, uint8_t byte);

/**
 * @brief   Pop a byte from the ring buffer.
 * @param   rb      Source buffer.
 * @param   byte    Pointer to store the dequeued byte.
 * @return  0 on success, -1 if the buffer is empty.
 */
int  ring_pop(ring_buf_t *rb, uint8_t *byte);

/**
 * @brief   Check whether the ring buffer is empty.
 * @param   rb  Buffer to query.
 * @return  Non-zero if empty.
 */
static inline int ring_is_empty(const ring_buf_t *rb) {
    return rb->count == 0;
}

/**
 * @brief   Check whether the ring buffer is full.
 * @param   rb  Buffer to query.
 * @return  Non-zero if full.
 */
static inline int ring_is_full(const ring_buf_t *rb) {
    return rb->count >= RING_BUF_SIZE;
}

#endif /* HAL_RING_H */
