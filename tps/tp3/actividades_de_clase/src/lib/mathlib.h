/**
 * @file    mathlib.h
 * @brief   Integer math, Q16.16 fixed-point arithmetic, bit manipulation,
 *          sine LUT, inline assembly utilities, and 2x2 matrix operations.
 *
 * Designed for embedded systems: no floating-point hardware required
 * (fixed-point), no heap allocation, all functions are reentrant.
 *
 * @note    Inline assembly sections are guarded by architecture macros
 *          (__x86_64__, __riscv, etc.) with a portable C fallback.
 */

#ifndef MATHLIB_H
#define MATHLIB_H

#include <stdint.h>
#include <stddef.h>

/* ── Fixed-point Q16.16 arithmetic ─────────────────────────────── */

/** @brief Q16.16 fixed-point type (16 integer bits, 16 fractional bits). */
typedef int32_t fixed16_t;

/** @brief Number of fractional bits in the Q16.16 format. */
#define FIXED16_SHIFT   16

/** @brief The value 1.0 in Q16.16 representation. */
#define FIXED16_ONE     (1 << FIXED16_SHIFT)

/** @brief The value 0.5 in Q16.16 representation. */
#define FIXED16_HALF    (1 << (FIXED16_SHIFT - 1))

/** @brief Convert an integer to Q16.16 fixed-point. */
#define INT_TO_FIXED(x)   ((fixed16_t)((x) << FIXED16_SHIFT))

/** @brief Convert a Q16.16 fixed-point value to integer (truncates). */
#define FIXED_TO_INT(x)   ((int)((x) >> FIXED16_SHIFT))

/** @brief Convert a float to Q16.16 fixed-point. */
#define FLOAT_TO_FIXED(x) ((fixed16_t)((x) * FIXED16_ONE))

/** @brief Convert a Q16.16 fixed-point value to float. */
#define FIXED_TO_FLOAT(x) ((float)(x) / FIXED16_ONE)

/**
 * @brief   Multiply two Q16.16 fixed-point values.
 * @param   a   First operand.
 * @param   b   Second operand.
 * @return  Product in Q16.16 format.
 */
fixed16_t fixed_mul(fixed16_t a, fixed16_t b);

/**
 * @brief   Divide two Q16.16 fixed-point values.
 * @param   a   Dividend.
 * @param   b   Divisor (returns 0 on division by zero).
 * @return  Quotient in Q16.16 format.
 */
fixed16_t fixed_div(fixed16_t a, fixed16_t b);

/* ── Integer math ──────────────────────────────────────────────── */

/**
 * @brief   Compute n! (iterative).
 * @param   n   Non-negative integer.
 * @return  Factorial of @p n. Overflows silently for large @p n.
 */
int32_t  math_factorial(int32_t n);

/**
 * @brief   Compute the n-th Fibonacci number (iterative).
 * @param   n   Index (0-based).
 * @return  F(n).
 */
int32_t  math_fibonacci(int32_t n);

/**
 * @brief   Greatest common divisor via Euclid's algorithm.
 * @param   a   First integer (sign is ignored).
 * @param   b   Second integer (sign is ignored).
 * @return  GCD(|a|, |b|).
 */
int32_t  math_gcd(int32_t a, int32_t b);

/**
 * @brief   Integer square root (Newton's method).
 * @param   n   Non-negative integer.
 * @return  Floor of the square root of @p n.
 */
uint32_t math_isqrt(uint32_t n);

/**
 * @brief   Primality test (trial division, 6k +/- 1 optimization).
 * @param   n   Integer to test.
 * @return  Non-zero if @p n is prime, 0 otherwise.
 */
int      math_is_prime(int32_t n);

/**
 * @brief   Clamp a value to the range [lo, hi].
 * @param   val Value to clamp.
 * @param   lo  Lower bound.
 * @param   hi  Upper bound.
 * @return  Clamped value.
 */
static inline int32_t math_clamp(int32_t val, int32_t lo, int32_t hi) {
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

/**
 * @brief   Absolute value.
 * @param   x   Input integer.
 * @return  |x|.
 */
static inline int32_t math_abs(int32_t x) {
    return (x < 0) ? -x : x;
}

/**
 * @brief   Return the smaller of two integers.
 * @param   a   First value.
 * @param   b   Second value.
 * @return  min(a, b).
 */
static inline int32_t math_min(int32_t a, int32_t b) {
    return (a < b) ? a : b;
}

/**
 * @brief   Return the larger of two integers.
 * @param   a   First value.
 * @param   b   Second value.
 * @return  max(a, b).
 */
static inline int32_t math_max(int32_t a, int32_t b) {
    return (a > b) ? a : b;
}

/* ── Bit manipulation ──────────────────────────────────────────── */

/**
 * @brief   Set a single bit in a register value.
 * @param   reg     Current register value.
 * @param   bit     Bit position (0-31).
 * @return  Register value with bit @p bit set.
 */
static inline uint32_t bit_set(uint32_t reg, uint8_t bit) {
    return reg | (1U << bit);
}

/**
 * @brief   Clear a single bit in a register value.
 * @param   reg     Current register value.
 * @param   bit     Bit position (0-31).
 * @return  Register value with bit @p bit cleared.
 */
static inline uint32_t bit_clear(uint32_t reg, uint8_t bit) {
    return reg & ~(1U << bit);
}

/**
 * @brief   Toggle a single bit in a register value.
 * @param   reg     Current register value.
 * @param   bit     Bit position (0-31).
 * @return  Register value with bit @p bit toggled.
 */
static inline uint32_t bit_toggle(uint32_t reg, uint8_t bit) {
    return reg ^ (1U << bit);
}

/**
 * @brief   Test whether a specific bit is set.
 * @param   reg     Register value to test.
 * @param   bit     Bit position (0-31).
 * @return  1 if bit is set, 0 otherwise.
 */
static inline int bit_test(uint32_t reg, uint8_t bit) {
    return (reg >> bit) & 1U;
}

/**
 * @brief   Count the number of set bits (population count).
 * @param   x   Input value.
 * @return  Number of 1-bits in @p x.
 */
uint8_t  bit_count(uint32_t x);

/**
 * @brief   Reverse the bit order of an 8-bit value.
 * @param   x   Input byte.
 * @return  Bit-reversed byte.
 */
uint8_t  bit_reverse8(uint8_t x);

/**
 * @brief   Reverse the bit order of a 32-bit value.
 * @param   x   Input word.
 * @return  Bit-reversed 32-bit value.
 */
uint32_t bit_reverse32(uint32_t x);

/* ── Sine lookup table (Q16.16, 0..90 degrees, 1-degree steps) ── */

/** @brief Number of entries in the sine LUT (0 to 90 inclusive). */
#define SINE_LUT_SIZE 91

/** @brief Precomputed sine values in Q16.16 for 0..90 degrees. */
extern const fixed16_t sine_lut[SINE_LUT_SIZE];

/**
 * @brief   Compute sine via LUT with quadrant folding.
 * @param   degrees Angle in degrees (any integer, wraps modulo 360).
 * @return  sin(degrees) in Q16.16 format.
 */
fixed16_t math_sin(int32_t degrees);

/**
 * @brief   Compute cosine via LUT (delegates to math_sin).
 * @param   degrees Angle in degrees.
 * @return  cos(degrees) in Q16.16 format.
 */
fixed16_t math_cos(int32_t degrees);

/**
 * @defgroup asm_utils Inline Assembly Utilities
 * @brief   Architecture-specific intrinsics with portable C fallbacks.
 *
 * Three implementations guarded by architecture macros:
 *   - @c __x86_64__ / @c __i386__ : x86 (BSR, BSF, BSWAP, RDTSC, ...)
 *   - @c __riscv : RISC-V (Zbb extension where available)
 *   - fallback : portable C (no inline asm)
 *
 * @par RISC-V notes
 *   - clz/ctz/rol/ror/rev8 require the Zbb extension
 *     (compile with @c -march=rv32imac_zbb or @c rv64gc_zbb)
 *   - rdcycle is base ISA but may trap in user mode
 *   - fence is base ISA
 *   - pause hint (Zihintpause) is safe as NOP on cores without it
 * @{
 */

#if defined(__x86_64__) || defined(__i386__)
/* ═══════════════════ x86 / x86-64 ═══════════════════════════════ */

/**
 * @brief   Count leading zeros (CLZ) using x86 BSR instruction.
 * @param   x   Input value (returns 32 when x == 0).
 * @return  Number of leading zero bits.
 */
static inline uint32_t asm_clz(uint32_t x) {
    if (x == 0) return 32;
    uint32_t leading;
    __asm__ volatile ("bsrl %1, %0" : "=r" (leading) : "r" (x));
    return 31 - leading;
}

/**
 * @brief   Count trailing zeros (CTZ) using x86 BSF instruction.
 * @param   x   Input value (returns 32 when x == 0).
 * @return  Number of trailing zero bits.
 */
static inline uint32_t asm_ctz(uint32_t x) {
    if (x == 0) return 32;
    uint32_t trailing;
    __asm__ volatile ("bsfl %1, %0" : "=r" (trailing) : "r" (x));
    return trailing;
}

/**
 * @brief   Read the CPU timestamp counter (RDTSC).
 * @return  64-bit cycle count.
 */
static inline uint64_t asm_rdtsc(void) {
    uint32_t lo, hi;
    __asm__ volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

/**
 * @brief   Byte-swap a 32-bit value (big-endian <-> little-endian).
 * @param   x   Input value.
 * @return  Byte-swapped value.
 */
static inline uint32_t asm_bswap32(uint32_t x) {
    __asm__ volatile ("bswapl %0" : "=r" (x) : "0" (x));
    return x;
}

/**
 * @brief   Byte-swap a 16-bit value.
 * @param   x   Input value.
 * @return  Byte-swapped value.
 */
static inline uint16_t asm_bswap16(uint16_t x) {
    __asm__ volatile ("rorw $8, %0" : "=r" (x) : "0" (x));
    return x;
}

/**
 * @brief   Compiler memory barrier (prevents reordering across this point).
 */
static inline void asm_memory_barrier(void) {
    __asm__ volatile ("" ::: "memory");
}

/**
 * @brief   Single NOP instruction.
 */
static inline void asm_nop(void) {
    __asm__ volatile ("nop");
}

/**
 * @brief   PAUSE hint — reduces power in spin-wait loops.
 */
static inline void asm_pause(void) {
    __asm__ volatile ("pause");
}

/**
 * @brief   32-bit rotate left.
 * @param   val     Value to rotate.
 * @param   shift   Rotation amount (0-31).
 * @return  Rotated value.
 */
static inline uint32_t asm_rol32(uint32_t val, uint8_t shift) {
    __asm__ volatile ("roll %%cl, %0" : "=r" (val) : "0" (val), "c" (shift));
    return val;
}

/**
 * @brief   32-bit rotate right.
 * @param   val     Value to rotate.
 * @param   shift   Rotation amount (0-31).
 * @return  Rotated value.
 */
static inline uint32_t asm_ror32(uint32_t val, uint8_t shift) {
    __asm__ volatile ("rorl %%cl, %0" : "=r" (val) : "0" (val), "c" (shift));
    return val;
}

#elif defined(__riscv)
/* ═══════════════════ RISC-V ═════════════════════════════════════ */

#if defined(__riscv_zbb)
/* Zbb extension: native clz, ctz, rol, ror, rev8 instructions */

/** @copydoc asm_clz */
static inline uint32_t asm_clz(uint32_t x) {
    uint32_t result;
#if __riscv_xlen == 64
    __asm__ volatile ("clzw %0, %1" : "=r" (result) : "r" (x));
#else
    __asm__ volatile ("clz %0, %1"  : "=r" (result) : "r" (x));
#endif
    return result;
}

/** @copydoc asm_ctz */
static inline uint32_t asm_ctz(uint32_t x) {
    uint32_t result;
#if __riscv_xlen == 64
    __asm__ volatile ("ctzw %0, %1" : "=r" (result) : "r" (x));
#else
    __asm__ volatile ("ctz %0, %1"  : "=r" (result) : "r" (x));
#endif
    return result;
}

/** @copydoc asm_rol32 */
static inline uint32_t asm_rol32(uint32_t val, uint8_t shift) {
    uint32_t result;
#if __riscv_xlen == 64
    __asm__ volatile ("rolw %0, %1, %2" : "=r" (result) : "r" (val), "r" ((uint32_t)shift));
#else
    __asm__ volatile ("rol %0, %1, %2"  : "=r" (result) : "r" (val), "r" ((uint32_t)shift));
#endif
    return result;
}

/** @copydoc asm_ror32 */
static inline uint32_t asm_ror32(uint32_t val, uint8_t shift) {
    uint32_t result;
#if __riscv_xlen == 64
    __asm__ volatile ("rorw %0, %1, %2" : "=r" (result) : "r" (val), "r" ((uint32_t)shift));
#else
    __asm__ volatile ("ror %0, %1, %2"  : "=r" (result) : "r" (val), "r" ((uint32_t)shift));
#endif
    return result;
}

/** @copydoc asm_bswap32 */
static inline uint32_t asm_bswap32(uint32_t x) {
    uint32_t result;
    __asm__ volatile ("rev8 %0, %1" : "=r" (result) : "r" (x));
#if __riscv_xlen == 64
    result >>= 32;   /* rev8 on RV64 reverses all 8 bytes; shift to get upper 32 */
#endif
    return result;
}

/** @copydoc asm_bswap16 */
static inline uint16_t asm_bswap16(uint16_t x) {
    return (uint16_t)((x >> 8) | (x << 8));
}

#else /* RISC-V without Zbb — C fallback for bit-manipulation */

/** @copydoc asm_clz */
static inline uint32_t asm_clz(uint32_t x) {
    if (x == 0) return 32;
    uint32_t n = 0;
    if ((x & 0xFFFF0000) == 0) { n += 16; x <<= 16; }
    if ((x & 0xFF000000) == 0) { n +=  8; x <<=  8; }
    if ((x & 0xF0000000) == 0) { n +=  4; x <<=  4; }
    if ((x & 0xC0000000) == 0) { n +=  2; x <<=  2; }
    if ((x & 0x80000000) == 0) { n +=  1; }
    return n;
}

/** @copydoc asm_ctz */
static inline uint32_t asm_ctz(uint32_t x) {
    if (x == 0) return 32;
    uint32_t n = 0;
    if ((x & 0x0000FFFF) == 0) { n += 16; x >>= 16; }
    if ((x & 0x000000FF) == 0) { n +=  8; x >>=  8; }
    if ((x & 0x0000000F) == 0) { n +=  4; x >>=  4; }
    if ((x & 0x00000003) == 0) { n +=  2; x >>=  2; }
    if ((x & 0x00000001) == 0) { n +=  1; }
    return n;
}

/** @copydoc asm_rol32 */
static inline uint32_t asm_rol32(uint32_t val, uint8_t shift) {
    shift &= 31;
    return (val << shift) | (val >> (32 - shift));
}

/** @copydoc asm_ror32 */
static inline uint32_t asm_ror32(uint32_t val, uint8_t shift) {
    shift &= 31;
    return (val >> shift) | (val << (32 - shift));
}

/** @copydoc asm_bswap32 */
static inline uint32_t asm_bswap32(uint32_t x) {
    return ((x >> 24) & 0x000000FF) | ((x >>  8) & 0x0000FF00) |
           ((x <<  8) & 0x00FF0000) | ((x << 24) & 0xFF000000);
}

/** @copydoc asm_bswap16 */
static inline uint16_t asm_bswap16(uint16_t x) {
    return (uint16_t)((x >> 8) | (x << 8));
}

#endif /* __riscv_zbb */

/* RISC-V base ISA: rdcycle CSR, fence, nop, pause hint */

/** @copydoc asm_rdtsc */
static inline uint64_t asm_rdtsc(void) {
#if __riscv_xlen == 64
    uint64_t cycles;
    __asm__ volatile ("rdcycle %0" : "=r" (cycles));
    return cycles;
#else
    uint32_t lo, hi;
    __asm__ volatile ("rdcycleh %0" : "=r" (hi));
    __asm__ volatile ("rdcycle  %0" : "=r" (lo));
    return ((uint64_t)hi << 32) | lo;
#endif
}

/** @copydoc asm_memory_barrier */
static inline void asm_memory_barrier(void) {
    __asm__ volatile ("fence rw, rw" ::: "memory");
}

/** @copydoc asm_nop */
static inline void asm_nop(void) {
    __asm__ volatile ("nop");
}

/**
 * @brief   Zihintpause: encoded as FENCE with pred=W, succ=0.
 *          Safe as a NOP on cores without Zihintpause support.
 */
static inline void asm_pause(void) {
    __asm__ volatile (".insn i 0x0F, 0, x0, x0, 0x010");
}

#else
/* ═══════════════════ Portable C fallback ════════════════════════ */

/** @copydoc asm_clz */
static inline uint32_t asm_clz(uint32_t x) {
    if (x == 0) return 32;
    uint32_t n = 0;
    if ((x & 0xFFFF0000) == 0) { n += 16; x <<= 16; }
    if ((x & 0xFF000000) == 0) { n +=  8; x <<=  8; }
    if ((x & 0xF0000000) == 0) { n +=  4; x <<=  4; }
    if ((x & 0xC0000000) == 0) { n +=  2; x <<=  2; }
    if ((x & 0x80000000) == 0) { n +=  1; }
    return n;
}

/** @copydoc asm_ctz */
static inline uint32_t asm_ctz(uint32_t x) {
    if (x == 0) return 32;
    uint32_t n = 0;
    if ((x & 0x0000FFFF) == 0) { n += 16; x >>= 16; }
    if ((x & 0x000000FF) == 0) { n +=  8; x >>=  8; }
    if ((x & 0x0000000F) == 0) { n +=  4; x >>=  4; }
    if ((x & 0x00000003) == 0) { n +=  2; x >>=  2; }
    if ((x & 0x00000001) == 0) { n +=  1; }
    return n;
}

/** @copydoc asm_rdtsc */
static inline uint64_t asm_rdtsc(void) {
    return 0;  /* no cycle counter available */
}

/** @copydoc asm_bswap32 */
static inline uint32_t asm_bswap32(uint32_t x) {
    return ((x >> 24) & 0x000000FF) | ((x >>  8) & 0x0000FF00) |
           ((x <<  8) & 0x00FF0000) | ((x << 24) & 0xFF000000);
}

/** @copydoc asm_bswap16 */
static inline uint16_t asm_bswap16(uint16_t x) {
    return (uint16_t)((x >> 8) | (x << 8));
}

/** @copydoc asm_memory_barrier */
static inline void asm_memory_barrier(void) {
    __asm__ volatile ("" ::: "memory");
}

/** @copydoc asm_nop */
static inline void asm_nop(void) {
    __asm__ volatile ("nop");
}

/** @copydoc asm_pause */
static inline void asm_pause(void) {
    __asm__ volatile ("" ::: "memory");
}

/** @copydoc asm_rol32 */
static inline uint32_t asm_rol32(uint32_t val, uint8_t shift) {
    shift &= 31;
    return (val << shift) | (val >> (32 - shift));
}

/** @copydoc asm_ror32 */
static inline uint32_t asm_ror32(uint32_t val, uint8_t shift) {
    shift &= 31;
    return (val >> shift) | (val << (32 - shift));
}

#endif /* architecture selection */

/** @} */ /* end of asm_utils group */

/* ── 2x2 Matrix (integer, stack-allocated) ─────────────────────── */

/**
 * @brief   2x2 integer matrix stored in row-major order.
 */
typedef struct {
    int32_t m[2][2];    /**< Matrix elements: m[row][col]. */
} mat2_t;

/**
 * @brief   Set a matrix to the 2x2 identity.
 * @param   out     Output matrix.
 */
void    mat2_identity(mat2_t *out);

/**
 * @brief   Multiply two 2x2 matrices: out = a * b.
 * @param   out     Output matrix (must not alias @p a or @p b).
 * @param   a       Left operand.
 * @param   b       Right operand.
 */
void    mat2_multiply(mat2_t *out, const mat2_t *a, const mat2_t *b);

/**
 * @brief   Compute the determinant of a 2x2 matrix.
 * @param   m       Input matrix.
 * @return  det(m) = m[0][0]*m[1][1] - m[0][1]*m[1][0].
 */
int32_t mat2_determinant(const mat2_t *m);

/**
 * @brief   Print a 2x2 matrix to stdout in a formatted table.
 * @param   m       Input matrix.
 */
void    mat2_print(const mat2_t *m);

#endif /* MATHLIB_H */
