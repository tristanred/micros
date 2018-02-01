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

/**
 * Returns the current task that is running.
 */
struct task_t* ks_get_current();

/**
 * Suspend the current thread and schedule another thread to run.
 
 * This method is the first stage of the 2-stage context switching process.
 * The first stage saves the registers on the stack in assembly and calls the 
 * stage 2 method.
 */
extern void ks_suspend();

/**
 * Finishes the thread switching operation. Must not be called by any other 
 * function than ks_suspend because it depends on the ordering of the 
 * stack frames.
 */
void ks_suspend_stage2();

/**
 * Helper function to gather the saved registers by ks_suspend and return them.
 * This method must not be called by any other method than ks_suspend_stage2.
 */
extern struct regs_t ks_get_stacked_registers();

/**
 * Set the next thread to run. This method does not save the current thread
 * information. Threads should call ks_suspend if they want to be resumed later.
 */
void ks_activate(struct task_t* next);

/**
 * Finalize the control transfer to the new task using register switch.
 */
extern void ks_do_activate(struct task_t* next);

/**
 * Get the next thread that is scheduled to run.
 */
struct task_t* ks_get_next_thread();

/**
 * Create a new thread with the provided entry point. The entrypoint should be
 * the address of a function.
 */
struct task_t* ks_create_thread(uint32_t entrypoint);

// Preempt functions. These functions get called from the timer IRQ and can stop
// the current thread and pass execution to another.
BOOL ks_should_preempt_current();
struct task_t* ks_preempt_current(registers_t* from);

#endif
