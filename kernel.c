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
void kernel_main(multiboot_info_t* arg1, uint8_t* arg2, uint8_t* arg3)
{
    if(arg2 != 0 || arg3 != 0)
    {
        kErrorBeforeInit(99, "kernel_main was passed unexpected parameters.");
        
        return; // PANIC ??
    }
    
    kSetupLog(SERIAL_COM1_BASE);
    
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
    
    kmInitManager();

    fbInitialize();
    
    char* arr = kmKernelAlloc(sizeof(char) * 16);
    arr[0] = 'a';
    arr[1] = 'b';
    arr[2] = 'c';
    arr[3] = 'd';
    arr[4] = 'e';
    
    char* arr2 = kmKernelAlloc(sizeof(char) * 16);
    arr2[0] = 'z';
    arr2[1] = 'x';
    arr2[2] = 'y';
    arr2[3] = 'w';
    
    kWriteLog("Setting screen fill pattern.");
    
    char fbFill[80 * 25];
    
    for(int i = 0; i < (80 * 25); i++)
    {
        fbFill[i] = 'a';
    }
    
    fbFill[80 * 25] = '\0';
    
    fbPutString(fbFill);
    
    fbMoveCursor(5, 5);
    fbPutChar(' ');
        
    kWriteLog("Kernel End");
}
