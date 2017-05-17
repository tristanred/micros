#include "vga.h"

#include "io_func.h"

void do_stuff()
{
    // Read misc register
    unsigned char misc = inb(0x3cc);
    
    // Read the feature control register
    unsigned char fc = inb(0x3ca);
    
    // Read the input status #1 register
    unsigned char input1 = inb(0x3c2);
    
    // Read the input status #2 register
    unsigned char input2 = inb(0x3ba);
    
    // Set the current Misc Graphic register
    
    // Special register access
    
    // Put the sub-register address in the Address port
    outb(0x3ce, 0x06);
    outb(0x3cf, 0xF);
    // Check if data was written ok
    unsigned char resData = inb(0x3cf);
    
    
    int i = 0;
}