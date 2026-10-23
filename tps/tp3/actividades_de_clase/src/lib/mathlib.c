/**
 * @file    mathlib.c
 * @brief   Implementation of integer math, Q16.16 fixed-point arithmetic,
 *          bit manipulation, sine LUT, and 2x2 matrix operations.
 *
 * @see     mathlib.h for the public API and inline assembly utilities.
 */

#include "mathlib.h"
#include <stdio.h>

/* ── Fixed-point arithmetic ────────────────────────────────────── */

fixed16_t fixed_mul(fixed16_t a, fixed16_t b) {
    int64_t tmp = (int64_t)a * (int64_t)b;
    return (fixed16_t)(tmp >> FIXED16_SHIFT);
}

fixed16_t fixed_div(fixed16_t a, fixed16_t b) {
    if (b == 0) return 0;
    int64_t tmp = ((int64_t)a << FIXED16_SHIFT);
    return (fixed16_t)(tmp / b);
}

/* ── Integer math ──────────────────────────────────────────────── */

int32_t math_factorial(int32_t n) {
    int32_t result = 1;
    for (int32_t i = 2; i <= n; i++)
        result *= i;
    return result;
}

int32_t math_fibonacci(int32_t n) {
    if (n <= 0) return 0;
    if (n == 1) return 1;
    int32_t a = 0, b = 1;
    for (int32_t i = 2; i <= n; i++) {
        int32_t tmp = a + b;
        a = b;
        b = tmp;
    }
    return b;
}

int32_t math_gcd(int32_t a, int32_t b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b != 0) {
        int32_t tmp = b;
        b = a % b;
        a = tmp;
    }
    return a;
}

uint32_t math_isqrt(uint32_t n) {
    if (n == 0) return 0;
    uint32_t x = n;
    uint32_t y = (x + 1) / 2;
    while (y < x) {
        x = y;
        y = (x + n / x) / 2;
    }
    return x;
}

int math_is_prime(int32_t n) {
    if (n < 2) return 0;
    if (n < 4) return 1;
    if (n % 2 == 0 || n % 3 == 0) return 0;
    for (int32_t i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0)
            return 0;
    }
    return 1;
}

/* ── Bit manipulation ──────────────────────────────────────────── */

uint8_t bit_count(uint32_t x) {
    uint8_t count = 0;
    while (x) {
        count += x & 1;
        x >>= 1;
    }
    return count;
}

uint8_t bit_reverse8(uint8_t x) {
    x = ((x & 0x55) << 1) | ((x & 0xAA) >> 1);
    x = ((x & 0x33) << 2) | ((x & 0xCC) >> 2);
    x = (x << 4) | (x >> 4);
    return x;
}

uint32_t bit_reverse32(uint32_t x) {
    x = ((x & 0x55555555) << 1)  | ((x & 0xAAAAAAAA) >> 1);
    x = ((x & 0x33333333) << 2)  | ((x & 0xCCCCCCCC) >> 2);
    x = ((x & 0x0F0F0F0F) << 4)  | ((x & 0xF0F0F0F0) >> 4);
    x = ((x & 0x00FF00FF) << 8)  | ((x & 0xFF00FF00) >> 8);
    x = (x << 16) | (x >> 16);
    return x;
}

/* ── Sine LUT (Q16.16, values for 0..90 degrees) ──────────────── */

const fixed16_t sine_lut[SINE_LUT_SIZE] = {
        0,  1143,  2287,  3429,  4571,  5711,  6850,  7986,  9120, 10252,
    11380, 12504, 13625, 14742, 15854, 16961, 18064, 19160, 20251, 21336,
    22414, 23486, 24550, 25606, 26655, 27696, 28729, 29752, 30767, 31772,
    32768, 33753, 34728, 35693, 36647, 37589, 38521, 39440, 40347, 41243,
    42125, 42995, 43852, 44695, 45525, 46340, 47142, 47929, 48702, 49460,
    50203, 50931, 51643, 52339, 53019, 53683, 54331, 54963, 55577, 56175,
    56755, 57319, 57864, 58393, 58903, 59395, 59870, 60326, 60763, 61183,
    61583, 61965, 62328, 62672, 62997, 63302, 63589, 63856, 64103, 64331,
    64540, 64729, 64898, 65047, 65176, 65286, 65376, 65446, 65496, 65526,
    65536
};

fixed16_t math_sin(int32_t degrees) {
    degrees = degrees % 360;
    if (degrees < 0) degrees += 360;

    int sign = 1;
    if (degrees >= 180) {
        sign = -1;
        degrees -= 180;
    }
    if (degrees > 90)
        degrees = 180 - degrees;

    return sign * sine_lut[degrees];
}

fixed16_t math_cos(int32_t degrees) {
    return math_sin(degrees + 90);
}

/* ── 2x2 Matrix ───────────────────────────────────────────────── */

void mat2_identity(mat2_t *out) {
    out->m[0][0] = 1; out->m[0][1] = 0;
    out->m[1][0] = 0; out->m[1][1] = 1;
}

void mat2_multiply(mat2_t *out, const mat2_t *a, const mat2_t *b) {
    out->m[0][0] = a->m[0][0]*b->m[0][0] + a->m[0][1]*b->m[1][0];
    out->m[0][1] = a->m[0][0]*b->m[0][1] + a->m[0][1]*b->m[1][1];
    out->m[1][0] = a->m[1][0]*b->m[0][0] + a->m[1][1]*b->m[1][0];
    out->m[1][1] = a->m[1][0]*b->m[0][1] + a->m[1][1]*b->m[1][1];
}

int32_t mat2_determinant(const mat2_t *m) {
    return m->m[0][0]*m->m[1][1] - m->m[0][1]*m->m[1][0];
}

void mat2_print(const mat2_t *m) {
    printf("| %6d %6d |\n", (int)m->m[0][0], (int)m->m[0][1]);
    printf("| %6d %6d |\n", (int)m->m[1][0], (int)m->m[1][1]);
}
