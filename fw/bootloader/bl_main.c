/*
 * bl_main.c — TP4 (Bloque 4): lógica del BOOTLOADER
 *
 * COMPLETAR los TODO.
 *   - Hito 5: con los pasos (0) y (6) alcanza para arrancar el FW sin validar nada.
 *   - Hito 7: se agregan las validaciones (1) a (5) y bl_fail(). El CRC es tu
 *     str_crc8() del TP3: el Makefile compila la implementación local de lib/src/strutils.c.
 *
 * Reglas:
 *   - El BL NO usa variables globales (todo en el stack o const): bl.ld lo
 *     tiene que hacer cumplir.
 *   - El BL nunca deja REG_FW_STATUS en 0xFF (¿por qué?).
 *   - Para imprimir usen print_str() de fw/lib/src/print.c o bl_log_hex().
 */
#include <stdint.h>
#include "includes.h"   /* config.h (registros) + print_str() de print.c */
#include "fw_image.h"   /* header + vector table del FW (el contrato)    */
#include "strutils.h"   /* tu str_crc8() del TP3 */

/* ---- Símbolos que tiene que definir bootloader/bl.ld (salen de memory_map.ld) ----
 * Para el linker son direcciones; en C se usan con '&' (o como array),
 * igual que _sdata/_ebss en un startup. */
extern const fw_image_header_t __fw_slot_start;  /* ORIGIN(APP_ROM): acá espera al FW      */
extern const uint8_t           __fw_slot_size[]; /* LENGTH(APP_ROM): el valor ES la dirección */

/* ---- bl_start.S ---- */
void bl_jump(uint32_t sp, uint32_t entry) __attribute__((noreturn));
void bl_halt(void) __attribute__((noreturn));
void bl_main(void) __attribute__((noreturn));

/* ---- Marcas de avance en REG_FW_REPORT (como test_fw_status.c del TP2):
 *      el testbench no las imprime, pero se ven en las ondas (make wave) ---- */
#define BL_REPORT_START   0xB0070000u   /* el BL arrancó              */
#define BL_REPORT_IMG_OK  0xB0070100u   /* imagen del FW validada     */
#define BL_REPORT_JUMP    0xB0070200u   /* saltando al FW             */
#define BL_REPORT_ERROR   0xB007E000u   /* error; bits bajos = código */

enum bl_error {
    BL_ERR_MAGIC = 1,   /* no hay FW en el slot                     */
    BL_ERR_LOAD_ADDR,   /* el FW fue linkeado para otra dirección   */
    BL_ERR_SIZE,        /* image_size fuera de rango                */
    BL_ERR_CRC,         /* imagen corrupta                          */
    BL_ERR_ENTRY,       /* reset_handler fuera de la imagen         */
    BL_ERR_SP,          /* initial_sp fuera de la RAM / desalineado */
};

/* (YA ESTÁ HECHO) Arma "<msg>0xXXXXXXXX" en el stack y lo manda en UNA
 * transmisión de UART0 (el testbench imprime cada una como "UART: ..."). */
static void bl_log_hex(const char *msg, uint32_t val)
{
    char line[64];
    unsigned int n = 0;

    while (*msg != '\0' && n < sizeof(line) - 11u)
        line[n++] = *msg++;
    line[n++] = '0';
    line[n++] = 'x';
    for (int shift = 28; shift >= 0; shift -= 4)
        line[n++] = "0123456789ABCDEF"[(val >> shift) & 0xFu];
    line[n] = '\0';
    print_str(line);
}

/* TODO (hito 7): error -> que quede visible y que la simulación termine.
 *   - mensaje por UART (why);
 *   - REG_FW_REPORT = BL_REPORT_ERROR | code;
 *   - REG_FW_STATUS = 0xBAD;
 *   - y cortar con bl_halt() (¿por qué no alcanza con 0xBAD?). */
static void __attribute__((noreturn)) bl_fail(enum bl_error code, const char *why)
{
    (void)code;
    (void)why;
    bl_halt();
}

void bl_main(void)
{
    const fw_image_header_t *hdr = &__fw_slot_start;    /* header + vector table del FW */

    /* TODO (hito 5) (0) Avisar que el BL arrancó: REG_FW_STATUS = 0 (corriendo),
     *                    REG_FW_REPORT = BL_REPORT_START y un mensaje por UART. */

    /* TODO (hito 7) (1) magic == FW_IMAGE_MAGIC                       -> si no, BL_ERR_MAGIC
     *                (2) load_addr == dirección del slot               -> si no, BL_ERR_LOAD_ADDR
     *                (3) FW_HDR_SIZE <= image_size <= tamaño del slot  -> si no, BL_ERR_SIZE
     *                (4) str_crc8() de [slot + FW_IMAGE_CRC_START, slot + image_size)
     *                    == image_crc                                  -> si no, BL_ERR_CRC
     *                (5) initial_sp dentro de la RAM y alineado a 16, reset_handler
     *                    dentro de la imagen y alineado a 4            -> BL_ERR_SP / BL_ERR_ENTRY
     *   Pista: la RAM está en config.h (ADDR_START_DATA, ADDR_SIZE_DATA). */

    /* TODO (hito 5) (6) Leer initial_sp y reset_handler de la vector table del FW
     *                    (hdr->...), marcar BL_REPORT_JUMP y saltar con bl_jump(). */

    (void)hdr; (void)bl_log_hex;                      /* borrar al completar */
    bl_fail(BL_ERR_MAGIC, "[BL] TODO: bl_main() sin completar");   /* borrar al completar */
}
