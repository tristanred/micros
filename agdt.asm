extern gdtPointer
global loadGdt

loadGdt:
    lgdt[gdtPointer]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:fload

fload:
    ret                 ; was ret.asm
   
