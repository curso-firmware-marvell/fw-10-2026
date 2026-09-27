MEMORY
{
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 128K
  RAM   (rwx): ORIGIN = 0x00020000, LENGTH = 128K
}
ENTRY(_start)

SECTIONS
{
  .text : {
    *(.text*)
    *(.rodata*)
  } > FLASH

  .data : {
    *(.data*)
  } > RAM AT > FLASH

  .bss : {
    *(.bss*)
    *(COMMON)
  } > RAM

  . = ALIGN(4);
  _stack_top = ORIGIN(RAM) + LENGTH(RAM);
}