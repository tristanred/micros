#include "program_idle.h"

#include "common.h"
#include "idt.h"

int program_idle_main()
{
    enable_interrupts();
    
    while(TRUE)
    {
        
    }
    
    return 0;
}
