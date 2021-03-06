#include "timer.h"
#include "idt.h"
#include "kernel_log.h"
#include "io_func.h"
#include "framebuffer.h"
#include "keyboard.h"
#include "kernel.h"
#include "task.h"

uint32_t tick = 0;

static void timer_callback(registers_t regs)
{
    mscounter++;

    ks_update_task();

    tick++;
    if(tick >= CPU_WAKEUP_FREQ)
    {
        tick = 0;

        if(cpu_is_idle == TRUE)
        {
            regs.eip++;
            cpu_is_idle = FALSE;
        }
    }
}

void init_timer(uint32_t frequency)
{
   // Firstly, register our timer callback.
   register_interrupt_handler(IRQ0, &timer_callback);

   register_interrupt_handler(IRQ1, &keyboard_interrupt_handler);

   timer_freq = frequency;

   // The value we send to the PIT is the value to divide it's input clock
   // (1193180 Hz) by, to get our required frequency. Important to note is
   // that the divisor must be small enough to fit into 16-bits.
   uint32_t divisor = 1193180 / frequency;
   timer_div = divisor;

   // Send the command byte.
   outb(0x43, 0x36);

   // Divisor has to be sent byte-wise, so split here into upper/lower bytes.
   uint8_t l = (uint8_t)(divisor & 0xFF);
   uint8_t h = (uint8_t)( (divisor>>8) & 0xFF );

   // Send the frequency divisor.
   outb(0x40, l);
   outb(0x40, h);
}

void sleep(uint32_t t)
{
    uint32_t target = mscounter + t;

    while(mscounter <= target)
    {
        cpu_idle();
    }
}

uint32_t getmscount()
{
    return mscounter;
}

uint32_t get_timer_rate()
{
    return timer_div;
}
