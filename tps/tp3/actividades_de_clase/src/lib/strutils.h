/**
 * @file    strutils.h
 * @brief   Fixed-size string buffer, in-place string operations, tokenizer,
 *          and CRC-8 — all designed for embedded systems (no heap).
 *
 * Every function operates on caller-owned buffers with explicit size
 * limits, ensuring bounded memory usage suitable for constrained targets.
 */

#ifndef STRUTILS_H
#define STRUTILS_H

#include <stddef.h>
#include <stdint.h>

/* ── Constants ─────────────────────────────────────────────────── */

/** @brief Maximum length (including NUL) for string buffers. */
#define STR_MAX_LEN     128

/** @brief Maximum number of tokens returned by str_split(). */
#define STR_MAX_TOKENS  16

/** @brief Human-readable library version string. */
extern const char *const STRUTILS_VERSION;

/** @brief Short library name identifier. */
extern const char *const STRUTILS_LIBNAME;

/* ── Fixed-size string buffer ──────────────────────────────────── */

/**
 * @brief   Fixed-capacity string buffer (stack- or static-allocated).
 *
 * Tracks its own length so operations like append are O(1) in the
 * common case (no redundant strlen calls).
 */
typedef struct {
    char    data[STR_MAX_LEN];  /**< NUL-terminated character storage. */
    size_t  len;                /**< Current string length (excluding NUL). */
} strbuf_t;

/**
 * @brief   Initialise a string buffer to the empty string.
 * @param   buf     Buffer to initialise.
 */
void   strbuf_init(strbuf_t *buf);

/**
 * @brief   Replace buffer contents with a new string.
 * @param   buf     Target buffer.
 * @param   src     NUL-terminated source string (truncated if too long).
 */
void   strbuf_set(strbuf_t *buf, const char *src);

/**
 * @brief   Append a NUL-terminated string to the buffer.
 * @param   buf     Target buffer.
 * @param   src     String to append (truncated if it would overflow).
 * @return  Number of characters actually appended.
 */
size_t strbuf_append(strbuf_t *buf, const char *src);

/**
 * @brief   Append the decimal string representation of an integer.
 * @param   buf     Target buffer.
 * @param   value   Integer to format and append.
 * @return  Number of characters actually appended.
 */
size_t strbuf_append_int(strbuf_t *buf, int32_t value);

/**
 * @brief   Clear the buffer (reset to empty string).
 * @param   buf     Buffer to clear.
 */
void   strbuf_clear(strbuf_t *buf);

/**
 * @brief   Get a read-only pointer to the buffer's C string.
 * @param   buf     Buffer to query.
 * @return  Pointer to the internal NUL-terminated data.
 */
const char *strbuf_cstr(const strbuf_t *buf);

/* ── In-place string operations (all work on caller-owned buffers) */

/**
 * @brief   Convert a string to uppercase.
 * @param   dst     Destination buffer (may equal @p src).
 * @param   src     Source string.
 * @param   max_len Size of @p dst including space for NUL.
 */
void   str_to_upper(char *dst, const char *src, size_t max_len);

/**
 * @brief   Convert a string to lowercase.
 * @param   dst     Destination buffer (may equal @p src).
 * @param   src     Source string.
 * @param   max_len Size of @p dst including space for NUL.
 */
void   str_to_lower(char *dst, const char *src, size_t max_len);

/**
 * @brief   Reverse a string in place.
 * @param   buf     Buffer containing the string.
 * @param   len     Number of characters to reverse (not counting NUL).
 */
void   str_reverse(char *buf, size_t len);

/**
 * @brief   Trim leading and trailing whitespace.
 * @param   dst     Destination buffer.
 * @param   src     Source string.
 * @param   max_len Size of @p dst including space for NUL.
 * @return  Length of the trimmed string.
 */
size_t str_trim(char *dst, const char *src, size_t max_len);

/**
 * @brief   Safe string copy with explicit size limit.
 * @param   dst     Destination buffer.
 * @param   src     Source string.
 * @param   max_len Size of @p dst including space for NUL.
 * @return  Number of characters copied (excluding NUL).
 */
size_t str_copy(char *dst, const char *src, size_t max_len);

/* ── Non-modifying queries ─────────────────────────────────────── */

/**
 * @brief   Test whether @p s begins with @p prefix.
 * @return  Non-zero if @p s starts with @p prefix, 0 otherwise.
 */
int    str_starts_with(const char *s, const char *prefix);

/**
 * @brief   Test whether @p s ends with @p suffix.
 * @return  Non-zero if @p s ends with @p suffix, 0 otherwise.
 */
int    str_ends_with(const char *s, const char *suffix);

/**
 * @brief   Compute the length of a NUL-terminated string.
 * @param   s   Input string.
 * @return  Number of characters before the NUL terminator.
 */
size_t str_length(const char *s);

/**
 * @brief   Compare two strings for equality.
 * @return  Non-zero if the strings are identical, 0 otherwise.
 */
int    str_equal(const char *a, const char *b);

/* ── Tokenizer (no heap, uses caller-provided storage) ─────────── */

/**
 * @brief   Result of a str_split() operation.
 */
typedef struct {
    const char *tokens[STR_MAX_TOKENS]; /**< Pointers into the split buffer. */
    size_t      count;                  /**< Number of tokens found. */
} str_tokens_t;

/**
 * @brief   Split a string in place by replacing @p delimiter with NUL.
 * @param   out         Output token list.
 * @param   buf         Mutable string to split (modified in place).
 * @param   delimiter   Character to split on.
 * @return  Number of tokens found (also stored in @p out->count).
 */
size_t str_split(str_tokens_t *out, char *buf, char delimiter);

/* ── CRC-8 (common in embedded protocols) ──────────────────────── */

/**
 * @brief   Compute CRC-8 with polynomial 0x07 (SMBus / 1-Wire).
 * @param   data    Pointer to input data.
 * @param   len     Number of bytes.
 * @return  8-bit CRC value.
 */
uint8_t str_crc8(const uint8_t *data, size_t len);

#endif /* STRUTILS_H */
