/**
 * @file    main.c
 * @brief   Application entry point for the binutils embedded demo.
 *
 * Orchestrates the three demo sections (math, string, embedded) and
 * the C++ wrapper demo.  Supports a @c --version command-line flag.
 */

#include <stdio.h>
#include <string.h>
#include "demo.h"

/** @brief C++ wrapper demo (defined in cpp_wrapper.cpp, linked via extern "C"). */
extern void run_cpp_demo(void);

/** @brief Program name embedded in .rodata for binutils string analysis. */
static const char PROGRAM_NAME[]    = "binutils-embedded-demo";

/** @brief Semantic version string embedded in .rodata. */
static const char PROGRAM_VERSION[] = "2.0.0";

/** @brief Print a visual separator line to stdout. */
static void print_separator(void) {
    printf("\n==================================================\n");
}


int main(int argc, char *argv[]) {
    printf("=== %s v%s ===\n", PROGRAM_NAME, PROGRAM_VERSION);
    printf("Embedded-oriented binary analysis demo for GNU binutils\n");

    if (argc > 1 && strcmp(argv[1], "--version") == 0) {
        printf("Version: %s\n", PROGRAM_VERSION);
        return 0;
    }

    print_separator();
    printf("--- Math & Fixed-Point Library ---\n");
    demo_mathlib();

    print_separator();
    printf("--- String Utilities (static buffers) ---\n");
    demo_strutils();

    print_separator();
    printf("--- Embedded Patterns ---\n");
    demo_embedded();

    print_separator();
    run_cpp_demo();

    print_separator();
    printf("Program finished successfully.\n");
    return 0;
}
