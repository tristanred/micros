#ifndef PROC_H
#define PROC_H

#include "common.h"
#include "vector.h"
#include "idt.h"

enum task_state
{
    T_WAITING,
    T_RUNNING,
    T_SUSPENDED
};

struct regs_t
{
    uint32_t flags;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
};

struct task_t
{
    uint32_t entryAddr;
    struct regs_t regs;
    uint32_t stackAddr;
    enum task_state state;
};

regs_t ks_get_registers();

struct task_t* ks_get_current();

BOOL ks_should_preempt_current();

struct task_t* ks_preempt_current(registers_t* from);

void ks_switch_to(struct task_t* to);


#endif
