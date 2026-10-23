/**
 * @file    demo_str.c
 * @brief   Demonstration of the strutils subsystem.
 *
 * Exercises fixed-size string buffer, case conversion, reverse, trim,
 * split/tokenizer, prefix/suffix queries, string builder, and CRC-8.
 */

#include "demo.h"
#include "strutils.h"
#include <stdio.h>
#include <stdint.h>

void demo_strutils(void) {
    char upper[STR_MAX_LEN];
    char lower[STR_MAX_LEN];
    char trimmed[STR_MAX_LEN];
    char reversed[STR_MAX_LEN];

    const char *test = "  Hello, Embedded World!  ";

    printf("[str] Original : '%s'\n", test);

    str_trim(trimmed, test, sizeof(trimmed));
    printf("[str] Trimmed  : '%s'\n", trimmed);

    str_to_upper(upper, trimmed, sizeof(upper));
    printf("[str] Upper    : '%s'\n", upper);

    str_to_lower(lower, trimmed, sizeof(lower));
    printf("[str] Lower    : '%s'\n", lower);

    str_copy(reversed, trimmed, sizeof(reversed));
    str_reverse(reversed, str_length(reversed));
    printf("[str] Reversed : '%s'\n", reversed);

    char split_buf[64];
    str_copy(split_buf, "one:two:three:four", sizeof(split_buf));
    str_tokens_t tokens;
    str_split(&tokens, split_buf, ':');
    printf("[str] Split    : ");
    for (size_t i = 0; i < tokens.count; i++)
        printf("[%s] ", tokens.tokens[i]);
    printf("\n");

    printf("[str] starts_with('Hello','Hel') = %s\n",
           str_starts_with("Hello", "Hel") ? "true" : "false");
    printf("[str] ends_with('Hello','llo')   = %s\n",
           str_ends_with("Hello", "llo") ? "true" : "false");

    strbuf_t sb;
    strbuf_init(&sb);
    strbuf_append(&sb, "PI ~ 3, answer = ");
    strbuf_append_int(&sb, 42);
    printf("[str] Builder  : '%s'\n", strbuf_cstr(&sb));

    const uint8_t crc_data[] = "Hello";
    printf("[crc] CRC-8('Hello') = 0x%02X\n", (unsigned)str_crc8(crc_data, 5));

    printf("[str] Version  : %s\n", STRUTILS_VERSION);
}
