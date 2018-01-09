#ifndef PROC_H
#define PROC_H

#include "common.h"
#include "vector.h"
#include "idt.h"

struct proc;
struct thread_info;

enum proc_status
{
    RUNNING,
    STOPPED
};

struct proc
{
    uint16_t pid;
    char name[64];
    struct vector* threadlist;
    int current_thread_nb; // Index in threadlist of the current thread
    enum proc_status current_status;
};

enum thread_status
{
    WAITING,
    RUNNING,
    SUSPENDED,
    KILLED
};

struct thread_info
{
    uint16_t tid;
    enum thread_status current_status;
    registers_t saved_registers; // If a thread is suspended registers go here
};

#endif
