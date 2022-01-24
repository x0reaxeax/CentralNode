[BITS 64]

section .text

global sys_finit_module
global sys_delete_module

sys_finit_module:
    mov     eax, 313
    syscall
    ret

sys_delete_module:
    mov     eax, 176
    syscall
    ret
