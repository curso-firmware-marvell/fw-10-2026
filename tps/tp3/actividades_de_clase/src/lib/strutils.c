/**
 * @file    strutils.c
 * @brief   Implementation of fixed-size string buffer, in-place string
 *          operations, tokenizer, and CRC-8.
 *
 * @see     strutils.h for the public API.
 */

#include "strutils.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* ── Constants ─────────────────────────────────────────────────── */

const char *const STRUTILS_VERSION = "strutils v2.0.0 - Embedded String Utilities";
const char *const STRUTILS_LIBNAME = "libstrutils";

/* ── Fixed-size string buffer ──────────────────────────────────── */

void strbuf_init(strbuf_t *buf) {
    buf->data[0] = '\0';
    buf->len = 0;
}

void strbuf_set(strbuf_t *buf, const char *src) {
    size_t slen = strlen(src);
    if (slen >= STR_MAX_LEN)
        slen = STR_MAX_LEN - 1;
    memcpy(buf->data, src, slen);
    buf->data[slen] = '\0';
    buf->len = slen;
}

size_t strbuf_append(strbuf_t *buf, const char *src) {
    size_t remaining = STR_MAX_LEN - 1 - buf->len;
    size_t slen = strlen(src);
    if (slen > remaining)
        slen = remaining;
    memcpy(buf->data + buf->len, src, slen);
    buf->len += slen;
    buf->data[buf->len] = '\0';
    return slen;
}

size_t strbuf_append_int(strbuf_t *buf, int32_t value) {
    char tmp[12];
    int n = snprintf(tmp, sizeof(tmp), "%d", (int)value);
    if (n < 0) return 0;
    return strbuf_append(buf, tmp);
}

void strbuf_clear(strbuf_t *buf) {
    buf->data[0] = '\0';
    buf->len = 0;
}

const char *strbuf_cstr(const strbuf_t *buf) {
    return buf->data;
}

/* ── In-place string operations ────────────────────────────────── */

void str_to_upper(char *dst, const char *src, size_t max_len) {
    size_t i;
    for (i = 0; i < max_len - 1 && src[i] != '\0'; i++)
        dst[i] = (char)toupper((unsigned char)src[i]);
    dst[i] = '\0';
}

void str_to_lower(char *dst, const char *src, size_t max_len) {
    size_t i;
    for (i = 0; i < max_len - 1 && src[i] != '\0'; i++)
        dst[i] = (char)tolower((unsigned char)src[i]);
    dst[i] = '\0';
}

void str_reverse(char *buf, size_t len) {
    if (len == 0) return;
    size_t i = 0, j = len - 1;
    while (i < j) {
        char tmp = buf[i];
        buf[i] = buf[j];
        buf[j] = tmp;
        i++;
        j--;
    }
}

size_t str_trim(char *dst, const char *src, size_t max_len) {
    while (*src && isspace((unsigned char)*src))
        src++;

    size_t slen = strlen(src);
    while (slen > 0 && isspace((unsigned char)src[slen - 1]))
        slen--;

    if (slen >= max_len)
        slen = max_len - 1;
    memcpy(dst, src, slen);
    dst[slen] = '\0';
    return slen;
}

size_t str_copy(char *dst, const char *src, size_t max_len) {
    size_t i;
    for (i = 0; i < max_len - 1 && src[i] != '\0'; i++)
        dst[i] = src[i];
    dst[i] = '\0';
    return i;
}

/* ── Non-modifying queries ─────────────────────────────────────── */

int str_starts_with(const char *s, const char *prefix) {
    while (*prefix) {
        if (*s != *prefix)
            return 0;
        s++;
        prefix++;
    }
    return 1;
}

int str_ends_with(const char *s, const char *suffix) {
    size_t slen = strlen(s);
    size_t plen = strlen(suffix);
    if (plen > slen) return 0;
    return memcmp(s + slen - plen, suffix, plen) == 0;
}

size_t str_length(const char *s) {
    const char *p = s;
    while (*p) p++;
    return (size_t)(p - s);
}

int str_equal(const char *a, const char *b) {
    while (*a && *a == *b) {
        a++;
        b++;
    }
    return *a == *b;
}

/* ── Tokenizer ─────────────────────────────────────────────────── */

size_t str_split(str_tokens_t *out, char *buf, char delimiter) {
    out->count = 0;
    if (buf == NULL || buf[0] == '\0')
        return 0;

    out->tokens[out->count++] = buf;
    for (char *p = buf; *p != '\0'; p++) {
        if (*p == delimiter) {
            *p = '\0';
            if (out->count < STR_MAX_TOKENS)
                out->tokens[out->count++] = p + 1;
        }
    }
    return out->count;
}

/* ── CRC-8 (polynomial 0x07, used in SMBus, 1-Wire, etc.) ─────── */

uint8_t str_crc8(const uint8_t *data, size_t len) {
    uint8_t crc = 0x00;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x80)
                crc = (uint8_t)((crc << 1) ^ 0x07);
            else
                crc <<= 1;
        }
    }
    return crc;
}
