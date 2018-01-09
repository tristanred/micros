global loadIdt
global get_pagefault_addr

loadIdt:
    mov eax, [esp+4]
    lidt [eax]
    ret

get_pagefault_addr:
    mov eax, cr2
    ret
