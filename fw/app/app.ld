/*
 * app.ld -- TP4 (Bloque 4): linker script del FW del TP anterior cuando corre
 * detras del bootloader. Partan de ../linker.ld y de la diapo 4.
 *
 * COMPLETAR los TODO (hito 3).
 */
ENTRY(Reset_Handler)

INCLUDE memory_map.ld

SECTIONS
{
  .text : {
    /* TODO: __fw_start y el header + vector table del FW (seccion .fw_header
     *       de app_startup.S) PRIMERO, al inicio de APP_ROM. ¿Por que KEEP? */
    *(.text .text.*)
    *(.rodata .rodata.*)
    /* TODO: ¿que otra seccion de solo lectura genera GCC en RISC-V? (hito 1) */
    . = ALIGN(4);
  } > APP_ROM

  /* TODO: .data
   *   - VMA en RAM, LMA dentro de APP_ROM (AT>)
   *   - simbolos _sdata / _edata (VMA) y _sidata (LMA, ver LOADADDR)
   *   - incluir tambien .sdata (globales chicas)                          */

  /* TODO: .bss
   *   - en RAM, sin ocupar lugar en la imagen (NOLOAD)
   *   - simbolos _sbss / _ebss
   *   - incluir tambien .sbss y COMMON                                    */

  /DISCARD/ : { *(.eh_frame .eh_frame_hdr) }
}

/* TODO: simbolos que usa el header (fw_image.h) y el arranque:
 *   __fw_end   = fin de la imagen cargable (¿donde termina la copia de .data?)
 *   __fw_size  = __fw_end - __fw_start
 *   _stack_top = tope de la RAM
 * TODO: ASSERT que verifique que .data + .bss dejan lugar para el stack. */
