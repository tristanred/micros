#include "program_idle.h"

#include "common.h"
#include "idt.h"

int program_idle_main()
{
    while(TRUE)
    {
        sleep(1);
    }
    
    return 0;
}
