#include "task_tests.h"

void task1()
{
    asm volatile("sti");
    
    int incr = 0;
    while(TRUE)
    {
        incr++;

        if(incr > 1000)
        {
            //Debugger();

            ks_suspend();
        }
    }
};

void task2()
{
    asm volatile("sti");
    
    int incr = 0;
    while(TRUE)
    {
        incr++;

        if(incr > 1000)
        {
            //Debugger();

            ks_suspend();
        }
    }
};


void test_do_tasks_tests()
{
    struct task_t* t1 = ks_create_thread((uint32_t)&task1);
    struct task_t* t2 = ks_create_thread((uint32_t)&task2);

    ks_activate(t1); // Start the test functions now.

}
