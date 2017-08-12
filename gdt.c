#include "gdt.h"

gdtEntry_t gdt[5];

/*
 * Access byte
 * Bit :    | 7 | 6 5 | 4  | 3 2 1 0 |
 * Content: | P | DPL | DT |  TYPE   |
 *
 * P = Segment present bit, 1 for yes
 * DPL = Descriptor privilege level, Ring 0 to 3
 * DT = Descriptor type. 0 for system, 1 for code or data.
 * TYPE = Code and data segment type. See Intel Vol. 3A 3-12
 *
 * Usual values :
 * 0x9A = 1 0 0 1           Kernel mode
 *                 1 0 1 0  Code segment Execute/Read
 * 0x92 = 1 0 0 1           Kernel mode
 *                 0 0 1 0  Data segment Read/Write
 * 0xFA = 1 1 1 1           User mode
 *                 1 0 1 0  Code segment Execute/Read
 * 0xF2 = 1 1 1 1           User mode
 *                 0 0 1 0  Data segment Read/Write
 *
 * Granularity byte
 * Bit :    | 7 | 6 | 5 | 4 | 3 2 1 0 |
 * Content: | G | D | 0 | A |    0    |
 *
 * G = Granularity. 0 = 1 byte, 1 = 1kb
 * D = Operand Size, 0 = 16bit, 1 = 32bit
 * A = Available.
 *
 * Usual values :
 * 0xCF = 1 1 0 0           4KB pages
 *                 1 1 1 1  Not used
 */
void gdtSetGate(int num, uint64_t base, uint64_t limit, uint8_t access, uint8_t gran)
{
    gdt[num].lBase      = (base & 0xFFFF);
    gdt[num].mBase      = (base >> 16) & 0xFF;
    gdt[num].hBase      = (base >> 24) & 0xFF;

    gdt[num].lLimit      = (limit & 0xFFFF);
    gdt[num].granularity   = ((limit >> 16) & 0x0F);

    gdt[num].granularity   |= (gran & 0xF0);
    gdt[num].access      = access;
}

void setupGdt()
{
    gdtPointer.limit   = (sizeof(gdtEntry_t) * 3) - 1;
    gdtPointer.base      = (uint32_t)&gdt;

    gdtSetGate(0, 0, 0, 0, 0); // Null segment
    gdtSetGate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Kernel Code
    gdtSetGate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Kernel Data
    gdtSetGate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User Code
    gdtSetGate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User Data

    loadGdt();
}
