global loadIdt

loadIdt:
    mov eax, [esp+4]
    lidt [eax]
    ret
