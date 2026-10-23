        .section .data
msg:
        .ascii "Hello from assembly!\n"
        .set msg_len, . - msg

answer:
        .long 42

        .section .bss
        .lcomm buffer, 256

        .section .text
        .globl asm_add
        .type asm_add, @function
asm_add:
        /* int asm_add(int a, int b) - adds two integers */
        movl    %edi, %eax
        addl    %esi, %eax
        ret
        .size asm_add, . - asm_add

        .globl asm_mul
        .type asm_mul, @function
asm_mul:
        /* int asm_mul(int a, int b) - multiplies two integers */
        movl    %edi, %eax
        imull   %esi, %eax
        ret
        .size asm_mul, . - asm_mul

        .globl asm_negate
        .type asm_negate, @function
asm_negate:
        /* int asm_negate(int a) - negates an integer */
        movl    %edi, %eax
        negl    %eax
        ret
        .size asm_negate, . - asm_negate

        .section .note.GNU-stack,"",@progbits
