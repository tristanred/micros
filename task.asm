global ks_get_registers
global ks_do_activate

global ks_do_taskstuff

extern Debugger

ks_get_registers: ; TODO 
    call ks_get_eip
    push 0 ; CS
    pushf ; Eflags
    pusha
    ret

ks_get_eip:
    mov eax, [esp]
    ret

ks_do_activate: ; p *(struct task_t*)$eax
    push ebp
    mov ebp, esp
    
    call Debugger
    
    mov eax, [ebp+8]
    
    mov ebx, [eax+28] ; Target stack
    
    mov ebx, [eax]
    mov ebx, [eax+4]
    mov ebx, [eax+8]
    mov ebx, [eax+12]
 
   ; Order of POPA
;    EDI ← Pop();
;    ESI ← Pop();
;    EBP ← Pop();
;    Increment ESP by 4; (* Skip next 4 bytes of stack *)
;    EBX ← Pop();
;    EDX ← Pop();
;    ECX ← Pop();
;    EAX ← Pop();

    popa
    popf
    pop cs
    
    pop ebp ; Prolly not need that
    ret

ks_do_taskstuff:
    int 0x80
    ret
