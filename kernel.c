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
    
    kSetupLog(SERIAL_COM1_BASE);

    kmInitManager();

    kWriteLog("***** Kernel Init *****");
    
    kWriteLog_format1d("WASD %d egugugug", 1234);
    
    /*
     * The kernel_main method currently receives the Multiboot info structure
     * from the boot.s code, contained in register EBX with the help of
     * the boot loader. The other arguments are to test the presence of
     * other parameters pushed into the method, helpful for debugging.
     */
    btmConfigureMemoryRanges(arg1);

    setupGdt();
    setupIdt();
    
    // asm volatile ("int $0x3");
    // asm volatile ("int $0x4");

    fbInitialize();
    
    asm volatile("sti");
    init_timer(1000);

    vector* vec = vector_create();
    
    vector_add(vec, 1);
    vector_add(vec, 2);
    vector_add(vec, 3);
    vector_add(vec, 4);
    vector_add(vec, 5);
    
    void* x = vector_get_at(vec, 10);
    
    SetupKeyboardDriver(SCANCODE_SET1);
    
    fbMoveCursor(0, 0);
        
    asm volatile ("int $0x3");
    asm volatile ("int $0x4");
    
    term_init();
    
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
