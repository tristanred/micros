#include "task.h"

#include "timer.h"
#include "kernel.h"
#include "program_idle.h"

void init_kernel_scheduler(struct kernel_info_block* kinfo)
{
    sched = alloc_kernel_module(sizeof(struct kernel_scheduler_module));
    kinfo->m_scheduler = sched;


    sched->processes = vector_create();
    sched->lastpid = 1;
    sched->sys_proc = NULL;

    sched->ts = kmalloc(sizeof(struct threadset));
    sched->ts->list = vector_create();
    sched->ts->critical_list = vector_create();
    sched->current = NULL;
    sched->currentIndex = 0;

    sched->idle_thread = NULL;

    sched->max_run_time = 200;
}

struct proc_t* ks_create_proc(const char* name, uint32_t entrypoint)
{
    struct proc_t* proc = (struct proc_t*)kmalloc(sizeof(struct proc_t));

    strncpy(proc->name, name, 32);
    proc->pid = ks_gen_pid();
    proc->threadsList = vector_create();

    struct task_t* t = ks_create_thread(entrypoint);
    vector_add(proc->threadsList, t);

    vector_add(sched->processes, proc);

    return proc;
}

void ks_create_system_proc()
{
    if(sched->sys_proc != NULL)
    {
        BUG("System proc called to initialize twice.");

        return;
    }

    struct proc_t* proc = (struct proc_t*)kmalloc(sizeof(struct proc_t));

    strncpy(proc->name, "System", 7);
    proc->pid = 0;
    proc->threadsList = vector_create();

    // Get the idle thread and create the thread
    struct task_t* idle_thread = ks_create_idle_task();
    vector_add(proc->threadsList, idle_thread);

    sched->sys_proc = proc;
}

uint32_t ks_gen_pid()
{
    sched->lastpid++;
    return sched->lastpid;
}

struct task_t* ks_get_current()
{
    return sched->current;
}

BOOL ks_get_task_index(struct task_t* task, size_t* index)
{
    struct vector* tasks = sched->ts->list;

    for(size_t i = 0; i < tasks->count; i++)
    {
        struct task_t* t = (struct task_t*)vector_get_at(tasks, i);

        if(t == task)
        {
            *index = i;

            return TRUE;
        }
    }

    return FALSE;
}

void ks_suspend_stage2()
{
    struct regs_t myregs = ks_get_stacked_registers();

    struct task_t* t = ks_get_current();

    /* Few things to suspend a thread
     * 1. Save the registers
     * 2. Mark thread as suspended
     * 3. Put it ad the end of the waitlist
     */
    t->regs = myregs;

    t->state = T_SUSPENDED;

    // Tell scheduler to pull up the next thread
    uint32_t nextIndex = 0;
    struct task_t* next = ks_get_next_thread(&nextIndex);
    ks_activate(next);
}

void ks_activate(struct task_t* next)
{
    sched->current->state = T_SUSPENDED;
    sched->current->ms_count_running = 0;

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
    newTask->ms_count_running = 0;
    newTask->ms_count_total = 0;

    newTask->entryAddr = entrypoint;
    newTask->state = T_WAITING;
    newTask->priority = T_PNORMAL;

    // Set the task to use the kernel heap.
    newTask->task_heap = kernel_info->m_memory_manager->kernel_heap;

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

    // Add the thread to the scheduler list.
    vector_add(sched->ts->list, newTask);

    return newTask;
}

struct task_t* ks_get_next_thread(uint32_t* nextIndex)
{
    // Main scheduling function

    // If an critical task has been found in ks_should_preempt_current
    // switch to it
    if(sched->ts->next_task != NULL)
    {
        struct task_t* t = sched->ts->next_task;
        sched->ts->next_task = NULL;

        size_t next = 0;
        BOOL foundTask = ks_get_task_index(t, &next);

        ASSERT(foundTask, "Sleeping task is not in the threadlist.");

        *nextIndex = next;

        return t;
    }

    // If any threads are sleeping, find one to wake up
    struct task_t* t = ks_get_sleeping_task();
    if(t != NULL)
    {
        size_t next = 0;
        BOOL foundTask = ks_get_task_index(t, &next);

        ASSERT(foundTask, "Sleeping task is not in the threadlist.");

        *nextIndex = next;

        return t;
    }

    // If no critical tasks are waiting or sleeping threads pending
    // just take the next index from the list.
    size_t next = (sched->currentIndex + 1) % sched->ts->list->count;
    t = vector_get_at(sched->ts->list, next);

    *nextIndex = next;

    return t;
}

void ks_update_task()
{
    struct task_t* c = ks_get_current();

    uint32_t timer_tickrate = get_timer_rate();
    c->ms_count_total += timer_tickrate;
    c->ms_count_running += timer_tickrate;
}

BOOL ks_should_preempt_current()
{
    struct task_t* c = ks_get_current();

    BOOL timeout = c->ms_count_running > sched->max_run_time;
    BOOL hasOtherTasks = sched->ts->list->count > 1;

    struct task_t* more_important_task = ks_priority_task_waiting();

    if(hasOtherTasks && more_important_task != NULL)
    {
        sched->ts->next_task = more_important_task;

        return TRUE;
    }

    if(
        timeout &&
        hasOtherTasks)
    {
        Debugger();

        return TRUE;
    }

    return FALSE;
}

struct task_t* ks_preempt_current(registers_t* from)
{
    struct task_t* currentTask = ks_get_current();
    currentTask->state = T_SUSPENDED;
    currentTask->regs.eax = from->eax;
    currentTask->regs.ecx = from->ecx;
    currentTask->regs.edx = from->edx;
    currentTask->regs.ebx = from->ebx;
    currentTask->regs.esp = from->esp + 16;
    currentTask->regs.ebp = from->ebp;
    currentTask->regs.esi = from->esi;
    currentTask->regs.edi = from->edi;
    currentTask->regs.flags = from->eflags;
    currentTask->entryAddr = from->eip; // TODO : Replace

    currentTask->ms_count_running = 0;

    uint32_t nextIndex = 0;
    struct task_t* nextTask = ks_get_next_thread(&nextIndex);
    nextTask->state = T_RUNNING;

    sched->current = nextTask;
    sched->currentIndex = nextIndex;

    return nextTask;
}

struct task_t* ks_create_idle_task()
{
    if(sched->ts->idle_task != NULL)
        return NULL;
    struct task_t* t = ks_create_thread((uint32_t)&program_idle_main);
    t->priority = T_PLOW;

    sched->ts->idle_task = t;
    return t;
}

BOOL ks_has_asleep_tasks()
{
    struct vector* tasks = sched->ts->list;

    for(size_t i = 0; i < tasks->count; i++)
    {
        struct task_t* t = (struct task_t*)vector_get_at(tasks, i);

        size_t currentSystemTime = getmscount();
        if(t->state == T_SLEEPING && t->ms_sleep_until > currentSystemTime)
            return TRUE;
    }

    return FALSE;
}

struct task_t* ks_get_sleeping_task()
{
    struct vector* tasks = sched->ts->list;

    for(size_t i = 0; i < tasks->count; i++)
    {
        struct task_t* t = (struct task_t*)vector_get_at(tasks, i);

        size_t currentSystemTime = getmscount();
        if(t->state == T_SLEEPING && t->ms_sleep_until > currentSystemTime)
            return t;
    }

    return NULL;
}

BOOL ks_can_wake_task(struct task_t* task)
{
    return task->state == T_SLEEPING && task->ms_sleep_until > getmscount();
}

void ks_criticalize_task(struct task_t* task)
{
    task->priority = T_PCRITICAL;

    vector_add(sched->ts->critical_list, task);
}

void ks_decriticalize_task(struct task_t* task)
{
    task->priority = T_PNORMAL; // TODO : Arbitrary conversion

    vector_remove(sched->ts->critical_list, task);
}

struct task_t* ks_priority_task_waiting()
{
    struct vector* tasks = sched->ts->critical_list;

    struct task_t* c = ks_get_current();

    for(size_t i = 0; i < tasks->count; i++)
    {
        struct task_t* t = (struct task_t*)vector_get_at(tasks, i);

        if(c->priority != T_PCRITICAL &&
            ks_can_wake_task(t) == TRUE)
        {
            return t;
        }

    }

    return NULL;
}
