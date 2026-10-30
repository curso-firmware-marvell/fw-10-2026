/*
 * memory_map.ld  --  TP4 (Bloque 4): mapa de memoria compartido por BL y FW
 *
 * Es la UNICA fuente de verdad de "donde va cada cosa": lo incluyen
 * bootloader/bl.ld y app/app.ld, asi que el FW queda linkeado exactamente
 * en la direccion donde el BL lo va a buscar (la direccion le llega a
 * bl_main.c como simbolo del linker, no como un #define repetido).
 *
 * Tiene que ser coherente con ../project_config.def:
 *   ADDR_START_INST = 0x0000_0000   ADDR_SIZE_INST = 0x0002_0000  (128 KiB)
 *   ADDR_START_DATA = 0x0002_0000   ADDR_SIZE_DATA = 0x0002_0000  (128 KiB)
 *
 * La memoria de instrucciones (memory_i en ../rtl/mem_mux.v) se parte en dos:
 *
 *   0x0000_0000 +--------------------------+ <- PROGADDR_RESET (picorv32, top.v)
 *               | BL_ROM   4 KiB           |    bootloader (su vector table en 0x0)
 *   0x0000_1000 +--------------------------+ <- slot del FW: header + vector table
 *               | APP_ROM  124 KiB         |    FW del practico anterior
 *   0x0002_0000 +--------------------------+
 *               | RAM      128 KiB         |    .data / .bss / stack (BL y despues FW)
 *   0x0004_0000 +--------------------------+ <- registros (project_config.def)
 */
MEMORY
{
  BL_ROM  (rx)  : ORIGIN = 0x00000000, LENGTH = 0x00001000
  APP_ROM (rx)  : ORIGIN = 0x00001000, LENGTH = 0x0001F000
  RAM     (rwx) : ORIGIN = 0x00020000, LENGTH = 0x00020000
}
