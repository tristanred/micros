global set_paging
global enablePaging
; global invalidateEntry

set_paging:
    push ebp
    mov ebp, esp
    mov eax, [esp+8]
    mov cr3, eax
    mov esp, ebp
    pop ebp
    ret

enablePaging:
    push ebp
    mov ebp, esp
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    mov esp, ebp
    pop ebp
    ret

; Not working, using inline asm in memory.c instead
; invalidateEntry:
;     invlpg [esp + 8]
;     ret
