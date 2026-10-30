/*
 * app_startup.S — TP4 (Bloque 4): arranque del FW cuando corre detrás del BL.
 * Reemplaza a ../startup.S (que no se toca).
 *
 * COMPLETAR los TODO (hito 4).
 */
#include "fw_image.h"

#define REG_FW_STATUS_ADDR  0x00040000      /* project_config.def: REG_FW_STATUS */
#define FW_STATUS_PASS      0xFF

/* ------------------------------------------------------------------------ */
/*  1) Header + vector table del FW (app.ld lo pone al inicio de APP_ROM)    */
/* ------------------------------------------------------------------------ */
    .section .fw_header, "a"
    .balign 4
    .globl  __fw_header
    .type   __fw_header, @object
__fw_header:
    /* TODO: los 6 campos de fw_image.h, en orden (.word = 4 bytes en RISC-V).
     *   0x00 magic, 0x04 load_addr, 0x08 image_size, 0x0C image_crc,
     *   0x10 initial_sp, 0x14 reset_handler.
     *   ¿Cuáles salen de símbolos del linker? ¿Qué valor ponen en image_crc,
     *   si lo escribe tools/mkimage.py DESPUÉS de linkear? */
    .word   0                       /* 0x00 magic          (TODO) */
    .word   0                       /* 0x04 load_addr      (TODO) */
    .word   0                       /* 0x08 image_size     (TODO) */
    .word   0                       /* 0x0C image_crc      (TODO) */
    .word   0                       /* 0x10 initial_sp     (TODO) */
    .word   0                       /* 0x14 reset_handler  (TODO) */
    .size   __fw_header, . - __fw_header

/* ------------------------------------------------------------------------ */
/*  2) Reset_Handler: "De 0 a main" (diapos 5 y 6)                           */
/* ------------------------------------------------------------------------ */
    .section .text.Reset_Handler, "ax"
    .balign 4                       /* contrato: reset_handler alineado a 4 */
    .globl  Reset_Handler
    .type   Reset_Handler, @function
Reset_Handler:
    /* TODO:
     *   1) sp = _stack_top. ¿Hace falta si el BL ya lo cargó desde la vector table?
     *   2) copiar .data: palabra por palabra de _sidata (LMA) a _sdata.._edata (VMA).
     *   3) poner .bss en cero: _sbss.._ebss.
     *   4) call main
     *   5) si main volvió y REG_FW_STATUS no quedó en FW_STATUS_PASS: ebreak.
     *      (¿qué pasa con la simulación si no lo hacen?)
     */

1:  j       1b                      /* borrar al completar */
    .size   Reset_Handler, . - Reset_Handler
