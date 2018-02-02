global ks_suspend
global ks_get_stacked_registers
global ks_do_activate

extern Debugger
extern ks_suspend_stage2

; Pushes all registers on the stack, this must be done early in the process so
; we can save all register values before they are changed by the following code.
; We get the ESP, EBP and EIP from the previous stack frame so that when we 
; recover the task it will be on the next instruction right after ks_suspend.
ks_suspend: ; void ks_suspend(void)
    push ebp
    mov ebp, esp

    pushf           ; EFLAGS
    push DWORD 0x8  ; CS
    pusha           ; regs 8 DW

    push ebp            ; Previous frame ESP
    add DWORD [esp], 8  ; Adjust for the 2 frames that we pushed ebp's on
    
    push DWORD [ebp+4]  ; Return addr (EIP) 
    push DWORD [ebp]    ; Previous frame EBP

    call ks_suspend_stage2
    
    ; Technically we will never reach this point because we call ks_activate
    ; and go to another thread. The function ks_suspend never returns.
    add esp, 52
    pop esp
    ret

; This function should be called from ks_suspend_stage2, this is because 
; this function goes up 2 stack frames in search of the saved registers.
; We can't just get them here because they would have been changed by the time
; we get here. The alternative is to write the whole pipeline of task transfer
; in assemblyto avoid clobbering registers. We would still have to go back up
; to grab the original EBP and ESP so meh.
ks_get_stacked_registers: ; struct regs_t ks_get_stacked_registers(void)
    push ebp
    mov ebp, esp
    sub esp, 48 ; 44 byte for struct and 4 for pushed eax(?)

    ; Select two stackframes above, frame with the flags stacked
    mov ebx, DWORD [ebp]
    mov ebx, DWORD [ebx]
    
    mov eax, [ebp+8] ; Where we drop the data
    
    mov edx, [ebx-44] ; ESP
    mov [eax+24], edx
    
    mov edx, [ebx-48] ; EIP
    mov [eax], edx
    
    mov edx, [ebx-4] ; EFLAGS
    mov [eax+8], edx
    
    mov edx, [ebx-8] ; CS
    mov [eax+4], edx
    
    mov edx, [ebx-40] ; EDI
    mov [eax+12], edx
    
    mov edx, [ebx-36] ; ESI
    mov [eax+16], edx
    
    mov edx, [ebx-52] ; EBP
    mov [eax+20], edx
    
    mov edx, [ebx-24] ; EBX
    mov [eax+28], edx
    
    mov edx, [ebx-20] ; EDX
    mov [eax+32], edx
    
    mov edx, [ebx-16] ; ECX
    mov [eax+36], edx
    
    mov edx, [ebx-12] ; EAX
    mov [eax+40], edx
    
    add esp, 48
    
    pop ebp
    ret

ks_do_activate: ; void ks_do_activate(struct task_t*)
    push ebp
    mov ebp, esp

    mov eax, [ebp+8]

    mov ebx, [eax+28] ; Target stack

    ;  ---> $ebp+4 <--- contains the saved EIP
    ; Move all the registers to target stack
    ; SUB target ESP to top of target stack
    ; Switch current stack to target stack
    ; pop registers back
    ; pop flags
    ; pop cs
    ; execute ret, which will pop eip

    ; Registers :   EAX = target task_t structure
    ;               EBX = target stack address
    ;               ECX = scratch register

    mov ecx, [eax+44] ; Emplace EAX
    mov [ebx-12], ecx

    mov ecx, [eax+40] ; Emplace ECX
    mov [ebx-16], ecx

    mov ecx, [eax+36] ; Emplace EDX
    mov [ebx-20], ecx

    mov ecx, [eax+32] ; Emplace EBX
    mov [ebx-24], ecx

    mov ecx, [eax+28] ; Emplace ESP STUB
    mov [ebx-28], DWORD 0 
    ; ESP not needed because we manually switch ESP rather than using the
    ; popped value.

    mov ecx, [eax+24] ; Emplace EBP
    mov [ebx-32], ecx

    mov ecx, [eax+20] ; Emplace ESI
    mov [ebx-36], ecx

    mov ecx, [eax+16] ; Emplace EDI
    mov [ebx-40], ecx

    mov ecx, [eax+12] ; Emplace EFLAGS
    mov [ebx-8], ecx

    ; TODO : Cannot switch CS during normal switching operations
    ; mov ecx, [eax+8] ; Emplace CS
    ; mov [ebx-52], ecx

    mov ecx, [eax+4] ; Emplace EIP
    mov [ebx-4], ecx

    mov ebp, [eax+24] ; Use the new ebp
    mov esp, [eax+28] ; Use the new stack

    sub esp, 40

    popa
    popfd

    ;pop ebp ; Not needed because we manually place it back
    ret
