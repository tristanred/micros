#include "gdt.h"

gdtEntry_t gdt[3];

void gdtSetGate(int num, uint64_t base, uint64_t limit, uint8_t access, uint8_t gran){
   gdt[num].lBase      = (base & 0xFFFF);
   gdt[num].mBase      = (base >> 16) & 0xFF;
   gdt[num].hBase      = (base >> 24) & 0xFF;

   gdt[num].lLimit      = (limit & 0xFFFF);
   gdt[num].granularity   = ((limit >> 16) & 0x0F);

   gdt[num].granularity   |= (gran & 0xF0);
   gdt[num].access      = access;
}

void setupGdt(){
   gdtPointer.limit   = (sizeof(gdtEntry_t) * 3) - 1;
   gdtPointer.base      = (uint32_t)&gdt;

   gdtSetGate(0, 0, 0, 0, 0);
   gdtSetGate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
   gdtSetGate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

   loadGdt();
}
