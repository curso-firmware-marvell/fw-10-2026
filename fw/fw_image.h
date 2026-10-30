/*
 * fw_image.h — TP4 (Bloque 4): formato del header + vector table del FW.
 *
 * Es el "contrato binario" entre el bootloader y el firmware, y es parte de lo
 * que da el TP (no se cambia: tools/mkimage.py y tools/check_load.py lo replican
 * en Python). Lo usan:
 *   - el BL (C):                 lo lee y lo valida antes de saltar
 *   - el arranque del FW (asm):  lo emite al principio de la imagen del FW
 *   - tools/mkimage.py:          escribe el CRC-8 después de linkear
 *
 * El FW empieza, en el slot ORIGIN(APP_ROM) = 0x0000_1000, con:
 *
 *   off   campo          lo completa
 *   0x00  magic          app_startup.S   ("TP4F")
 *   0x04  load_addr      linker          (__fw_start: dónde fue linkeado)
 *   0x08  image_size     linker          (__fw_size, ver app/app.ld)
 *   0x0C  image_crc      tools/mkimage.py (post-link): CRC-8 en el byte bajo
 *   ---- vector table del FW: DIRECCIONES, como en Cortex-M ----
 *   0x10  initial_sp     linker          (_stack_top)
 *   0x14  reset_handler  linker          (Reset_Handler)
 *
 * El CRC es el del TP3: str_crc8() de bloque3/trabajo_practico/src/lib/strutils.c
 * (polinomio 0x07, valor inicial 0xFF). Cubre [load_addr + 0x10, load_addr + image_size):
 * vector table + código + .rodata + valores iniciales de .data.
 */
#ifndef FW_IMAGE_H
#define FW_IMAGE_H

/* Solo #defines arriba: este archivo también lo incluye código assembler (.S) */
#define FW_IMAGE_MAGIC        0x46345054   /* bytes 'T','P','4','F' en memoria (little-endian) */

#define FW_HDR_MAGIC          0x00
#define FW_HDR_LOAD_ADDR      0x04
#define FW_HDR_IMAGE_SIZE     0x08
#define FW_HDR_IMAGE_CRC      0x0C
#define FW_HDR_INITIAL_SP     0x10
#define FW_HDR_RESET_HANDLER  0x14
#define FW_HDR_SIZE           0x18

#define FW_IMAGE_CRC_START    FW_HDR_INITIAL_SP  /* el CRC arranca en la vector table */
#define FW_IMAGE_CRC_UNSET    0xFFFFFFFF         /* valor en app.elf antes de mkimage.py
                                                    (ningún CRC-8 vale eso) */

#ifndef __ASSEMBLER__
#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t magic;
    uint32_t load_addr;
    uint32_t image_size;
    uint32_t image_crc;
    /* vector table del FW */
    uint32_t initial_sp;
    uint32_t reset_handler;
} fw_image_header_t;

/* El layout del struct es parte de la ABI BL<->FW (orden, padding, alineación):
 * si alguien lo cambia sin actualizar los offsets, no compila. */
_Static_assert(sizeof(fw_image_header_t) == FW_HDR_SIZE, "fw_image_header_t: tamaño");
_Static_assert(offsetof(fw_image_header_t, load_addr)     == FW_HDR_LOAD_ADDR,     "offset load_addr");
_Static_assert(offsetof(fw_image_header_t, image_size)    == FW_HDR_IMAGE_SIZE,    "offset image_size");
_Static_assert(offsetof(fw_image_header_t, image_crc)     == FW_HDR_IMAGE_CRC,     "offset image_crc");
_Static_assert(offsetof(fw_image_header_t, initial_sp)    == FW_HDR_INITIAL_SP,    "offset initial_sp");
_Static_assert(offsetof(fw_image_header_t, reset_handler) == FW_HDR_RESET_HANDLER, "offset reset_handler");
#endif /* __ASSEMBLER__ */

#endif /* FW_IMAGE_H */
