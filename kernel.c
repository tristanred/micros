#include <stddef.h>
#include <stdint.h>

/* Check if the compiler thinks we are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif


/**
 * These are the default includes in GCC. These headers are always available
 * even when compiling in --freestanding mode.
 *
 * #include <float.h>       http://www.cplusplus.com/reference/cfloat/
 * #include <iso646.h>      http://www.cplusplus.com/reference/ciso646/
 * #include <limits.h>      http://www.cplusplus.com/reference/climits/
 * #include <stdarg.h>      http://www.cplusplus.com/reference/cstdarg/
 * #include <stdbool.h>     http://www.cplusplus.com/reference/cstdbool/
 * #include <stddef.h>      http://www.cplusplus.com/reference/cstddef/
 * #include <stdint.h>      http://www.cplusplus.com/reference/cstdint/
 */

#include "kernel.h"

#include "kernel_features.h"

#include "multiboot.h"

#include "bootmem.h"
#include "framebuffer.h"
#include "serial.h"
#include "kernel_log.h"
#include "memory.h"
#include "gdt.h"
#include "idt.h"
#include "timer.h"
#include "keyboard.h"
#include "common.h"
#include "string.h"
#include "vector.h"
#include "error.h"
#include "terminal.h"
#include "pci.h"
#include "ata_driver.h"
#include "filesystem.h"
#include "array_utils.h"
#include "ezfs.h"
#include "ksh.h"

uint32_t kErrorBad;
char* kBadErrorMessage;

void kErrorBeforeInit(uint32_t errno, char* msg)
{
    // Do something with the error code and gtfo
 
    kErrorBad = errno;

    kBadErrorMessage = msg;
}

extern void _cpu_idle();

struct regs_t
{
    uint32_t flags;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
};

enum task_state
{
    T_WAITING,
    T_RUNNING,
    T_SUSPENDED
};

struct task_t
{
    uint32_t entryAddr;
    struct regs_t regs;
    uint32_t stackAddr;
    enum task_state state;
};

#define TASK_LEN 2
int currenttask;
struct task_t tasks[TASK_LEN];

struct task_t* get_switch_state(registers_t* from)
{
    int currentTaskIndex = currenttask;
    int nextTaskIndex = (currenttask + 1) % TASK_LEN;
    
    struct task_t* currentTask = &(tasks[currenttask]);
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
    currentTask->entryAddr = from->eip;
    
    struct task_t* nextTask = &(tasks[nextTaskIndex]);
    nextTask->state = T_RUNNING;
    
    currenttask = nextTaskIndex;
    from = currentTask;
    
    //Debugger();
    
    return nextTask;
}

BOOL should_switch_task()
{
    return FALSE;
}

uint8_t t1_stack[1024*1];
void task1()
{
    int incr = 0;
    while(TRUE)
    {
        incr++;
        
        if(incr > 1000)
        {
            ks_suspend();
        }
    }
};

uint8_t t2_stack[1024*1];
void task2()
{
    int incr = 0;
    while(TRUE)
    {
        incr++;
        
        if(incr > 1000)
        {
            ks_suspend();
        }
    }
};

void setup_tasks()
{
    currenttask = 0;
    
    struct task_t* t1 = &(tasks[0]);
    t1->regs.eax = 0;
    t1->regs.ebp = t1_stack + 1024;
    t1->regs.ebx = 0;
    t1->regs.ecx = 0;
    t1->regs.edi = 0;
    t1->regs.edx = 0;
    t1->regs.esi = 0;
    t1->regs.esp = t1_stack + 1024;
    t1->regs.flags = 0x202;
    t1->entryAddr = &task1;
    t1->stackAddr = t1_stack + 1024;
    t1->state = T_WAITING;
    
    struct task_t* t2 = &(tasks[1]);
    t2->regs.eax = 0;
    t2->regs.ebp = t2_stack + 1024;
    t2->regs.ebx = 0;
    t2->regs.ecx = 0;
    t2->regs.edi = 0;
    t2->regs.edx = 0;
    t2->regs.esi = 0;
    t2->regs.esp = t2_stack + 1024;
    t2->regs.flags = 0x202;
    t2->entryAddr = &task2;
    t2->stackAddr = t2_stack + 1024;
    t2->state = T_WAITING;
};

extern void ks_do_taskstuff();

#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif
void kernel_main(multiboot_info_t* arg1)
{
    cpu_is_idle = FALSE;
    panic = FALSE;

    setupGdt();
    setupIdt();

    kSetupLog(SERIAL_COM1_BASE);

    init_page_allocator();
    //pa_test_paging();
    
    init_memory_manager();

    //      TEST ZONE
    init_kernel_scheduler();
    
    //Debugger();
    struct task_t* t1 = ks_create_thread(&task1);
    struct task_t* t2 = ks_create_thread(&task2);
    
    ks_activate(t1);
    Debugger();
    
    setup_tasks();
    //init_timer(1000);
    
    Debugger();
    asm volatile("sti");
    
    task1();
    
    while(TRUE)
    {
        cpu_idle();
    }

    Debugger();
    
    //      TEST ZONE
    
    char* memTest = (char*)kmallocf(128, MEM_CHECKED);
    memTest[129] = 'b';
    free(memTest); // Will detect overflow
    char* memTest2 = (char*)kmalloc(128);
    char* memTest3 = (char*)kmalloc(128);
    char* memTest4 = (char*)kmalloc(128);

    setup_kernel_block();

    fbInitialize();

    kSetupLog(SERIAL_COM1_BASE);

    init_memory_manager();

    //setup_paging();

    //test_paging();

    init_module_kernel_features(kernel_info);
    // init_module_memory_manager(kernel_info);
    init_module_ata_driver(kernel_info);

    kfDetectFeatures(arg1);

    // PCI bus scanning
    int total = 0;

    struct pci_controlset* set = get_devices_list(&total);

    for(int i = 0; i < total; i++)
    {
        kWriteLog("");
        kWriteLog_format1d("Device #%d", i);
        print_pci_device_info(set->deviceList[i]);
    }

    setup_filesystem();
    ezfs_prepare_disk();

    // file_h file = ezfs_create_file(ROOT_DIR, "test.txt", FS_READ_WRITE, FS_FLAGS_NONE);

    // char filebuf[4];
    // strcpy(filebuf, "abcd");

    // size_t bytesWritten = ezfs_write_file(file, (uint8_t*)filebuf, 4);

    // uint8_t* outBuf = NULL;
    // size_t readBytes = ezfs_read_file(file, &outBuf);

    // fbPutString((char*)outBuf);
    // ASSERT(bytesWritten == readBytes, "WRONG SIZE WRITTEN.");

    // Enable interrupts
    asm volatile("sti");

    // Example of interrupts calls.
    // asm volatile ("int $0x3");
    // asm volatile ("int $0x4");

    init_timer(1000);
    SetupKeyboardDriver(SCANCODE_SET1);

    ksh_take_fb_control();

#ifdef MM_ENABLE_HEAP_ALLOC_CANARY
    BOOL res = mm_verify_all_allocs_canary();

    if(res == FALSE)
    {
        Debugger();
    }
#endif

    while(TRUE)
    {
        ksh_update();

        cpu_idle();
    }

    kWriteLog("Kernel End");
}

void setup_kernel_block()
{
    kernel_info = (struct kernel_info_block*)(KIBLOCK_ADDR_START);

    kernel_info->modules_start_address = (uint32_t)(kernel_info + sizeof(struct kernel_info_block));
    kernel_info->modules_end_address = (uint32_t)(kernel_info + (1024*1024*5)); // 5MB
    kernel_info->modules_current_offset = kernel_info->modules_start_address;
}

void* alloc_kernel_module(size_t size)
{
    //return malloc(size);

    if(!has_free_modules_space())
        return NULL; // TODO : Panic instead, unrecoverable situation

    uint32_t nextModuleAddress = kernel_info->modules_current_offset + size;
    kernel_info->modules_current_offset += size;

    return (void*)nextModuleAddress;
}

BOOL has_free_modules_space()
{
    return kernel_info->modules_start_address + kernel_info->modules_current_offset < kernel_info->modules_end_address;
}

void cpu_idle()
{
    cpu_is_idle = TRUE;
    _cpu_idle();
}
