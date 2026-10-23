/**
 * @file    demo.h
 * @brief   Entry-point declarations for each demonstration section.
 *
 * Each demo function exercises a specific subsystem and prints its
 * results to stdout, forming the test harness for GNU binutils analysis.
 */

#ifndef DEMO_H
#define DEMO_H

/**
 * @brief   Demonstrate the math library: factorial, fibonacci, GCD, isqrt,
 *          primes, fixed-point arithmetic, sine LUT, bit ops, and matrix.
 */
void demo_mathlib(void);

/**
 * @brief   Demonstrate the string utilities: trim, case conversion, reverse,
 *          split, prefix/suffix tests, string builder, and CRC-8.
 */
void demo_strutils(void);

/**
 * @brief   Demonstrate embedded HAL patterns: UART, ring buffer, ISR,
 *          external assembly functions, and inline assembly intrinsics.
 */
void demo_embedded(void);

#endif /* DEMO_H */
