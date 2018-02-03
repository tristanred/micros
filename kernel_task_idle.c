#include "kernel_task_idle.h"

size_t idleCycles = 0;

int kernel_task_idle_main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
    
    while(TRUE)
    {
        idleCycles++;
        
        cpu_idle();
    }
}
