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
#include "disk.h"
#include "pci.h"
#include "ata_driver.h"

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
    panic = FALSE;
    
    setupGdt();
    setupIdt();

    kSetupLog(SERIAL_COM1_BASE);    
    kmInitManager();
    
    setup_paging();
    
    char* far_address = (char*)0x3C00000; // 60 MB
    char* close_address = (char*)0xF00000; // 15 MB
    
    strcpy(far_address, "far_address\0");
    strcpy(close_address, "close_address\0");
    
    map_phys_address(0x3C00000, 0xF00000); // Map 60 MB mark to 15 MB mark.
    
    // Write the string to the address 0x3C00000, which goes over to 0xF00000
    // So far_address still have the old 'far_address' string.
    strcpy(far_address, "xx_far_address_after_mapping\0");
    
    // Both addresses should have the same content since they are mapped to the 
    // same page.
    int res = strcmp(close_address, far_address) == 0;
    ASSERT(res == TRUE, "PAGING IS FUCKED UP");
    
    setup_kernel_block();
    init_module_kernel_features(kernel_info);
    init_module_memory_manager(kernel_info);
    
    kWriteLog("***** Kernel Init *****");
    
    kfDetectFeatures(arg1);

    // kWriteLog("***** KERNEL MEMORY STATS *****");        
    // int total = 0;
    // char** x = kmGetMemoryStatsText(&total);
    
    // for(int i = 0; i < total; i++)
    // {
    //     kWriteLog(x[i]);
    // }

    /*
     * The kernel_main method currently receives the Multiboot info structure
     * from the boot.s code, contained in register EBX with the help of
     * the boot loader. The other arguments are to test the presence of
     * other parameters pushed into the method, helpful for debugging.
     */
    //btmConfigureMemoryRanges(arg1);

    //char* x = *(char*)0x06400000; // Generates a page fault.

    // asm volatile ("int $0x3");
    // asm volatile ("int $0x4");

    fbInitialize();
    
    struct pci_device result = get_device(0, 1, 1);
    print_pci_device_info(&result);
    
    test_io_port();
    
    int total = 0;
    struct pci_device** list = get_devices_list(&total);
    
    for(int i = 0; i < total; i++)
    {
        kWriteLog("");
        kWriteLog_format1d("Device #%d", i);
        print_pci_device_info(list[i]);
    }
    
    asm volatile("sti");
    init_timer(1000);
    
    SetupKeyboardDriver(SCANCODE_SET1);
    
    fbMoveCursor(0, 0);
        
    asm volatile ("int $0x3");
    asm volatile ("int $0x4");
    
    //term_init();
    
    //term_deactivate();
    
    //term_showSplashScreen();
    
    fbPutString("AAA");
    
    uint32_t cycles = 0;
    
    while(!panic)
    {
        // fbMoveCursor(0, 0);
        
        // char msstr[255];
        
        // sprintf_1d(msstr, "%d", mscounter);
        
        // fbPutString(msstr);
        
        cycles++;
    }
    
    kWriteLog("Kernel End");
}

void setup_kernel_block()
{
    kernel_info = (struct kernel_info_block*)0x100000;
    
    kernel_info->modules_start_address = (uint32_t)(&kernel_info + sizeof(struct kernel_info_block));
    kernel_info->modules_end_address = (uint32_t)(&kernel_info + (1024*1024*5)); // 5MB
    kernel_info->modules_current_offset = kernel_info->modules_start_address;
}

void* alloc_kernel_module(size_t size)
{
    if(!has_free_modules_space())
        return NULL;
    
    uint32_t nextModuleAddress = kernel_info->modules_current_offset + size;
    kernel_info->modules_current_offset += size;
    
    return (void*)nextModuleAddress;
}

BOOL has_free_modules_space()
{
    return kernel_info->modules_start_address + kernel_info->modules_current_offset < kernel_info->modules_end_address;
}
