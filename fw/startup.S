.section .text
.global _start

_start:
    la sp, _stack_top     # set stack pointer
    call main              # run C code
1:  j 1b                   # infinite loop