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
    
    
    int i = 0;
}