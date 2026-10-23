#ifndef MATHLIB_H
#define MATHLIB_H

#include <stdint.h>

int32_t  math_factorial(int32_t n);
int32_t  math_gcd(int32_t a, int32_t b);
uint32_t math_isqrt(uint32_t n);
int      math_is_prime(int32_t n);

static inline uint32_t bit_set(uint32_t reg, uint8_t bit) {
    return reg | (1U << bit);
}

#endif /* MATHLIB_H */
