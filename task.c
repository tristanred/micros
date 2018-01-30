#include "task.h"

void init_kernel_scheduler()
{
    sched = kmalloc(sizeof(struct kernel_scheduler_module));
    sched->ts = kmalloc(sizeof(struct threadset));
    sched->ts->list = vector_create();
    sched->current = NULL;
    sched->currentIndex = 0;
}

struct task_t* ks_get_current()
{
    return sched->current;
}

void ks_suspend()
{
    struct regs_t myregs = ks_save_fix_registers();
    
    BOOL returned = FALSE;

    struct task_t* t = ks_get_current();

    // This call represent the state at which the program will be
    // restored to.
    struct regs_t cr;
    ks_get_registers(&cr);
    
    if(returned == TRUE)
        return;

    returned = TRUE;

    /* Few things to suspend a thread
     * 1. Save the registers
     * 2. Mark thread as suspended
     * 3. Put it ad the end of the waitlist
     */
    t->regs = cr;

    t->state = T_SUSPENDED;

    // Tell scheduler to pull up the next thread
    struct task_t* next = ks_get_next_thread();
    ks_activate(next);
}

void ks_activate(struct task_t* next)
{
    sched->current = next;

    if(next->regs.eip == 0)
    {
        next->regs.eip = next->entryAddr;
    }

    /* To activate a thread
     * 1. Mark as RUNNING
     * 2. Load the saved registers
     *    At this point, we can't use the stack
     * 3. JMP to the saved EIP
     *    Or do 'push eax; ret'
     */

    ks_do_activate(next);

    // Open question
    // How to change the stack pointer and keep a ref to
    // the 'next' parameter to use EIP ?
    // Maybe call an interrupt ? Interrupt + replace stack to iret to new task
}

struct task_t* ks_create_thread(uint32_t entrypoint)
{
    struct task_t* newTask = (struct task_t*)kmalloc(sizeof(struct task_t));

    newTask->entryAddr = entrypoint;
    newTask->state = T_WAITING;

    size_t stackSize = 4096;
    newTask->stackAddr = (uint32_t)malloc(stackSize); // Create a stack
    newTask->regs.esp = newTask->stackAddr + 4096;

    // Setting the regs to testable values
    newTask->regs.eax = 1;
    newTask->regs.ecx = 2;
    newTask->regs.edx = 3;
    newTask->regs.ebx = 4;
    newTask->regs.ebp = newTask->regs.esp;
    newTask->regs.esi = 7;
    newTask->regs.edi = 8;
    newTask->regs.eip = 0;
    newTask->regs.cs = 10;
    newTask->regs.flags = 0;

    vector_add(sched->ts->list, newTask);

    return newTask;
}

struct task_t* ks_get_next_thread()
{
    size_t nextIndex = (sched->currentIndex + 1) % sched->ts->list->count;
    struct task_t* t = vector_get_at(sched->ts->list, nextIndex);

    sched->currentIndex = nextIndex;

    return t;
}
