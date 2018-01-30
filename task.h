#ifndef PROC_H
#define PROC_H

#include "common.h"
#include "vector.h"
#include "idt.h"
#include "vector.h"

enum task_state
{
    T_WAITING,
    T_RUNNING,
    T_SUSPENDED
};

struct regs_t
{
    uint32_t eip, cs, flags; // Added eip and cs
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Added by PUSHA
};

/*
typedef struct registers
{
   uint32_t ds;                  // Data segment selector
   uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
   uint32_t int_no, err_code;    // Interrupt number and error code (if applicable)
   uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} registers_t;
*/

// TODO : Convert regs to struct registers_t (change task switch ASM)
struct task_t // Size is 56 Bytes (unsure about task_state)
{
    uint32_t entryAddr;
    struct regs_t regs;
    uint32_t stackAddr;
    enum task_state state;
};

// List of threads managed by the kernel
struct threadset
{
    struct vector* list;
};

struct kernel_scheduler_module
{
    struct threadset* ts;
    struct task_t* current;
    uint32_t currentIndex;
};

struct kernel_scheduler_module* sched;

void init_kernel_scheduler();

extern struct regs_t ks_save_fix_registers();
extern void ks_get_registers(struct regs_t* regs);

// Get the EIP value of the call to this function
uint32_t ks_get_eip();

struct task_t* ks_get_current();

// Suspend the current thread and start the next one
void ks_suspend();

void ks_activate(struct task_t* next);
extern void ks_do_activate(struct task_t* next);

struct task_t* ks_get_next_thread();

void ks_switch_to(struct task_t* to);

struct task_t* ks_create_thread(uint32_t entrypoint);

// Preempt functions. These functions get called from the timer IRQ and can stop
// the current thread and pass execution to another.
BOOL ks_should_preempt_current();
struct task_t* ks_preempt_current(registers_t* from);




#endif
