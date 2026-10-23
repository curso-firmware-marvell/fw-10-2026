#ifndef STRUTILS_H
#define STRUTILS_H

#include <stddef.h>
#include <stdint.h>

#define STR_MAX_LEN 128

/** Valor inicial del CRC-8 en este laboratorio (las actividades de clase usan `0x00`). */
#define STR_CRC8_INIT 0xFF

typedef struct {
    char   data[STR_MAX_LEN];
    size_t len;
} strbuf_t;

void   strbuf_init(strbuf_t *buf);
void   strbuf_set(strbuf_t *buf, const char *src);
size_t strbuf_append(strbuf_t *buf, const char *src);
size_t strbuf_append_int(strbuf_t *buf, int32_t value);
const char *strbuf_cstr(const strbuf_t *buf);

size_t str_trim(char *dst, const char *src, size_t max_len);
uint8_t str_crc8(const uint8_t *data, size_t len);

#endif /* STRUTILS_H */
