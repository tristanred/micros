#include <stddef.h>
#include <stdint.h>

/* Check if the compiler thinks we are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
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

#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif
void kernel_main(multiboot_info_t* arg1)
{    
    cpu_is_idle = FALSE;
    panic = FALSE;
    
    setupGdt();
    setupIdt();

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
    struct pci_device** list = get_devices_list(&total);
    
    for(int i = 0; i < total; i++)
    {
        kWriteLog("");
        kWriteLog_format1d("Device #%d", i);
        print_pci_device_info(list[i]);
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
    kernel_info = (struct kernel_info_block*)0x100000;
    
    kernel_info->modules_start_address = (uint32_t)(kernel_info + sizeof(struct kernel_info_block));
    kernel_info->modules_end_address = (uint32_t)(kernel_info + (1024*1024*5)); // 5MB
    kernel_info->modules_current_offset = kernel_info->modules_start_address;
}

void* alloc_kernel_module(size_t size)
{
    return malloc(size);
    
    // if(!has_free_modules_space())
    //     return NULL;
    
    // uint32_t nextModuleAddress = kernel_info->modules_current_offset + size;
    // kernel_info->modules_current_offset += size;
    
    // return (void*)nextModuleAddress;
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