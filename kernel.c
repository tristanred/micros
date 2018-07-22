#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

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
#include "task.h"
#include "kernel_task_idle.h"
#include "bootlog.h"
#include "disk_manager.h"

uint32_t kErrorBad;
char* kBadErrorMessage;

void kErrorBeforeInit(uint32_t errno, char* msg)
{
    // Do something with the error code and gtfo

    kErrorBad = errno;

    kBadErrorMessage = msg;
}

#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif
void kernel_main(multiboot_info_t* arg1)
{
    // Standard Boot Process
    // This function is called by boot.asm once the bootloader is done.
    // We receive a multiboot_info_t structure in parameter describing the state
    // of the computer we're on.

    // We some flags to indicate the current state of the CPU.
    cpu_is_idle = FALSE;
    panic = FALSE;
    
    // The IDT and GDT tables need to be initialized. The CPU uses these tables
    // to direct interrupts to the kernel and to describe the memory layout
    // of the kernel.
    setupGdt();
    setupIdt();

    // We setup the COM port as early as possible to enable logging.
    // Enabling the COM port uses no other systems than IO ports.
    kSetupLog(SERIAL_COM1_BASE);

    // Initializes the Framebuffer, this will allow text rendering on the screen
    // This also does not use any systems other than IO ports.
    fbInitialize();
    fbClear();

    // Once logging and fb are online, we must setup the kernel block.
    // The Kernel Block is the root data structure that contains information
    // about the kernel. Right now, the main use is to contain a reference to
    // the structures of the kernel modules. The kernel block lives at
    // a static address MODULES_LOCATION declared in memory_zones.h.
    // The address must be static because we must setup the structure before we
    // have dynamic memory management.
    setup_kernel_block();
    kBootProgress("Kernel Block created\n");

    // Now we initialize the kernel modules. Each module is responsible for
    // a subsystem of the kernel. Things like IO, memory, threads are managed
    // by different modules.

    // The first module to initialize is the Page Allocator. This will provide
    // virtual memory with paging. Paging IS ACTIVATED once init_page_allocator
    // is called. We must now remember to allocate the page of memory we touch
    // or the kernel will panic. This is good to enable it first because we
    // will detect memory problems earlier in the development.
    init_page_allocator(kernel_info);
    kBootProgress("Page Allocator created\n");

    // The memory manager needs to be initialized soon in the process. This
    // system manages kernel and program heaps. This uses the multiboot
    // parameter to scan the memory zones reported by the BIOS. Once the
    // memory is partitioned we can have a kernel heap and field calls to
    // 'kmalloc'. This module does not require the page allocator to be
    // online but enabling paging after the memory manager is created is
    // counterproductive.
    init_memory_manager(kernel_info, arg1);
    kBootProgress("Memory Manager created\n");

    // The next important module to setup is the kernel scheduler. The KS
    // is responsible for managing processes and threads. Once active, the
    // scheduler stops the active thread and picks another one to run.
    // The scheduler uses the Timer interrupt to analyse if the current thread
    // should be stopped and to pick another one.
    init_kernel_scheduler(kernel_info);
    kBootProgress("Kernel Scheduler created\n");

    // The keyboard interrupt handler is automatically hooked in the init_timer
    // function so we need to make sure th initialize the driver to receive
    // the interrupts or else we'll hang the system.
    SetupKeyboardDriver(SCANCODE_SET1);
    
    // Register the interrupt handler for the timer chip. This will get us
    // a steady call every MS. The timer DOES NOT start ticking at this time.
    // Only when the interrupts are enabled will we receive the calls.
    // This sends an update to the kernel scheduler to update the running
    // task status.
    init_timer(TIMER_FREQ_1MS);
    kBootProgress("Timer setup for 1ms ticks.\n");

    // Globally enable the interrupts. This will start popping up the timer
    // and the scheduler will start switching to other threads periodically.
    enable_interrupts();
    kBootProgress("Interrupts ONLINE\n");


    // Create the System process. This is the process that will "own" the kernel
    // and do actual stuff. One of these things is starting the Idle thread.
    // This special thread is always active and sitting at the lowest priority.
    // This is so the kernel always have something to do.
    ks_create_system_proc();
    fbPutString("Just done the system proc !\n");
    
    //      TEST ZONE    
    Debugger();
    init_module_ata_driver(kernel_info);
    
    struct diskman* dm = create_diskman();
    
    struct disk d1;
    int res1 = connect_disk(dm, BUS_ATA, 0, &d1);
    
    struct disk d2;
    int res2 = connect_disk(dm, BUS_ATA, 1, &d2);
    
    struct disk d3;
    int res3 = connect_disk(dm, BUS_ATA, 2, &d3);
    
    struct disk d4;
    int res4 = connect_disk(dm, BUS_ATA, 3, &d4);
    
    res1 = disk_write(&d1, 0, "wasda", 5);
    
    uint8_t* buf = NULL;
    size_t reads = 0;
    res1 = disk_read(&d1, 0, 5, &buf, &reads);
    
    kWriteLog("PCI SCAN START\n");
    int total = 0;
    struct pci_controlset* set = get_devices_list(&total);

    for(int i = 0; i < total; i++)
    {
        kWriteLog("");
        kWriteLog("Device #%d", i);
        print_pci_device_info(set->deviceList[i]);
    }
    kWriteLog("PCI SCAN END\n");

    ksh_take_fb_control();
    
    while(TRUE)
    {
        ksh_update();
        
        cpu_idle();
    }

    //      TEST ZONE

    fbInitialize();

    kSetupLog(SERIAL_COM1_BASE);

    //init_memory_manager(kernel_info);

    //setup_paging();

    //test_paging();

    init_module_kernel_features(kernel_info);
    //init_module_memory_manager(kernel_info);

    kfDetectFeatures(arg1);

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
    enable_interrupts();

    // Example of interrupts calls.
    // asm volatile ("int $0x3");
    // asm volatile ("int $0x4");

    init_timer(TIMER_FREQ_1MS);

    ksh_take_fb_control();

    BOOL res = mm_verify_all_allocs_canary();

    if(res == FALSE)
    {
        Debugger();
    }

    while(TRUE)
    {
        ksh_update();

        cpu_idle();
    }

    kWriteLog("Kernel End");
}

void setup_kernel_block()
{
    kernel_info = (struct kernel_info_block*)(MODULES_LOCATION);

    kernel_info->modules_start_address = (uint32_t)((uint32_t)kernel_info + sizeof(struct kernel_info_block));
    kernel_info->modules_end_address = (uint32_t)((uint32_t)kernel_info + MODULES_LENGTH); // 1MB
    kernel_info->modules_current_offset = 0;
}

void* alloc_kernel_module(size_t size)
{
    if(!has_free_modules_space())
    {
        PanicQuit("Failed to allocate kernel module.");
        return NULL;
    }

    uint32_t nextModuleAddress = kernel_info->modules_start_address + kernel_info->modules_current_offset;
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
