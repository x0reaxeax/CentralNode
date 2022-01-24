; Not useful anymore, but I'm too lazy to remove it now, since it's literally everywhere.
; And I don't wanna hear anything about `sed`, so keep it
[BITS 64]

section .text

global __spaddr64

__spaddr64:
    mov     rax, [rsp]
    ret