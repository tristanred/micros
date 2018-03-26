#ifndef PROC_H
#define PROC_H

#include "common.h"
#include "vector.h"
#include "idt.h"
#include "vector.h"

enum task_state
{
    T_WAITING,          // Has not been run yet
    T_RUNNING,          // Currently running on the CPU
    T_SUSPENDED,        // Was stopped by another thread
    T_SLEEPING,         // Waiting for a specific time to wake up
    T_UNINTERRUPTIBLE   // Thread will not be preempted
};

enum task_prio
{
    T_PLOW,
    T_PNORMAL,
    T_PHIGH,
    T_PCRITICAL
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
    // Do not modify the order of the members above.
    // Assembly code depends on the correct ordering of the fields
    
    //TODO : ensure project runs with new fields in this struct (task.asm)
    enum task_prio priority;
    
    uint32_t ms_count_total; // Lifetime of the task
    uint32_t ms_count_running; // MS count since last suspended
    
    uint32_t ms_sleep_until; // Task will sleep until system hits this tick
};

// List of threads managed by the kernel
struct threadset
{
    struct vector* list;
    struct vector* critical_list;
    
    struct task_t* next_task;
    
    struct task_t* idle_task;
};

struct kernel_scheduler_module
{
    // Threads info
    struct threadset* ts;
    struct task_t* current;
    uint32_t currentIndex;
    
    // Scheduling info
    uint32_t max_run_time; // Time in ms given to a thread before we preempt it
};

struct kernel_scheduler_module* sched;

void init_kernel_scheduler(struct kernel_info_block* kinfo);

void ks_enable_scheduling();
void ks_disable_scheduling();

/**
 * Returns the current task that is running.
 */
struct task_t* ks_get_current();

/**
 * Get the index of a task from the threadset index.
 */
BOOL ks_get_task_index(struct task_t* task, size_t* index);

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
struct task_t* ks_get_next_thread(uint32_t* nextIndex);

/**
 * Create a new thread with the provided entry point. The entrypoint should be
 * the address of a function.
 * This function will schedule the task for execution.
 */
struct task_t* ks_create_thread(uint32_t entrypoint);

/**
 * Update function called every timer tick. This updates the scheduler and adds
 * running time to the current process.
 */
void ks_update_task();

// Preempt functions. These functions get called from the timer IRQ and can stop
// the current thread and pass execution to another.
BOOL ks_should_preempt_current();
struct task_t* ks_preempt_current(registers_t* from);

// Sleep functions TODO: Classify and document
void ks_create_idle_task();
BOOL ks_has_asleep_tasks();
struct task_t* ks_get_sleeping_task();

BOOL ks_can_wake_task(struct task_t* task);

// Priority functions

void ks_criticalize_task(struct task_t* task);
void ks_decriticalize_task(struct task_t* task);

struct task_t* ks_priority_task_waiting();

#endif
