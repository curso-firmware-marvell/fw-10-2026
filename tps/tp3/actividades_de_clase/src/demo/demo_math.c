/**
 * @file    demo_math.c
 * @brief   Demonstration of the mathlib subsystem.
 *
 * Exercises integer math, Q16.16 fixed-point arithmetic, sine/cosine LUT,
 * bit-manipulation helpers, and 2x2 matrix operations.  Output is printed
 * to stdout for validation and binutils string analysis.
 */

#include "demo.h"
#include "mathlib.h"
#include <stdio.h>

void demo_mathlib(void) {
    printf("[math] Factorial(10)  = %d\n", (int)math_factorial(10));
    printf("[math] Fibonacci(20)  = %d\n", (int)math_fibonacci(20));
    printf("[math] GCD(48, 18)    = %d\n", (int)math_gcd(48, 18));
    printf("[math] isqrt(65536)   = %u\n", (unsigned)math_isqrt(65536));

    printf("[math] Primes < 30    =");
    for (int i = 2; i < 30; i++) {
        if (math_is_prime(i))
            printf(" %d", i);
    }
    printf("\n");

    printf("[math] clamp(15,0,10) = %d\n", (int)math_clamp(15, 0, 10));

    printf("[fixed] 2.5 * 3.0     = %.4f\n",
           FIXED_TO_FLOAT(fixed_mul(FLOAT_TO_FIXED(2.5f), FLOAT_TO_FIXED(3.0f))));
    printf("[fixed] 10.0 / 3.0    = %.4f\n",
           FIXED_TO_FLOAT(fixed_div(FLOAT_TO_FIXED(10.0f), FLOAT_TO_FIXED(3.0f))));

    printf("[lut]   sin(0)        = %.4f\n",  FIXED_TO_FLOAT(math_sin(0)));
    printf("[lut]   sin(30)       = %.4f\n",  FIXED_TO_FLOAT(math_sin(30)));
    printf("[lut]   sin(90)       = %.4f\n",  FIXED_TO_FLOAT(math_sin(90)));
    printf("[lut]   cos(60)       = %.4f\n",  FIXED_TO_FLOAT(math_cos(60)));

    printf("[bits]  bit_count(0xFF)    = %u\n", (unsigned)bit_count(0xFF));
    printf("[bits]  bit_reverse8(0xA5) = 0x%02X\n", (unsigned)bit_reverse8(0xA5));
    printf("[bits]  bit_set(0, 3)      = 0x%08X\n", (unsigned)bit_set(0, 3));
    printf("[bits]  bit_test(0x08, 3)  = %d\n", bit_test(0x08, 3));

    mat2_t a = {{ {1, 2}, {3, 4} }};
    mat2_t b = {{ {5, 6}, {7, 8} }};
    mat2_t c;
    mat2_multiply(&c, &a, &b);
    printf("[mat2]  A * B:\n");
    mat2_print(&c);
    printf("[mat2]  det(A*B) = %d\n", (int)mat2_determinant(&c));
}
