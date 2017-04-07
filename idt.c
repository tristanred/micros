#include "idt.h"

#include "kernel_log.h"

idtEntry idtEntries[256];

 /*
  * Flags byte
  * Bit:     | 7 | 6 5 | 4 | 3 2 1 0 |
  * Content: | P | DPL | 0 |  TYPE   |
  *
  * P = Interrupt is present bit, 1 for working interrupt
  * DPL = Descriptor Privilege level, 0 for HW int and 3 for Software int
  * Type = Gate type. 0x8E for 32b int gate, 0x05 for Task, 0x0F for 32b Trap
  */
void idtSetEntry(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
    idtEntries[num].loffset = base & 0xFFFF;
    idtEntries[num].hoffset = (base >> 16) & 0xFFFF;
 
    idtEntries[num].selector     = sel;
    idtEntries[num].zero = 0;
    // We must uncomment the OR below when we get to using user-mode.
    // It sets the interrupt gate's privilege level to 3.
    idtEntries[num].typeattr   = flags /* | 0x60 */;
}

void setupIdt()
{
    idtPointer.limit = (sizeof(idtEntry) * 256) - 1;
    idtPointer.base = (uint32_t)&idtEntries;
    
    // Fill interrupt table
    idtSetEntry(0,(uint32_t)isr0, 0x08, 0x8E);
    idtSetEntry(1,(uint32_t)isr1, 0x08, 0x8E);
    idtSetEntry(2,(uint32_t)isr2, 0x08, 0x8E);
    idtSetEntry(3,(uint32_t)isr3, 0x08, 0x8E);
    idtSetEntry(4,(uint32_t)isr4, 0x08, 0x8E);
    idtSetEntry(5,(uint32_t)isr5, 0x08, 0x8E);
    idtSetEntry(6,(uint32_t)isr6, 0x08, 0x8E);
    idtSetEntry(7,(uint32_t)isr7, 0x08, 0x8E);
    idtSetEntry(8,(uint32_t)isr8, 0x08, 0x8E);
    idtSetEntry(9,(uint32_t)isr9, 0x08, 0x8E);
    idtSetEntry(10,(uint32_t)isr10, 0x08, 0x8E);
    idtSetEntry(11,(uint32_t)isr11, 0x08, 0x8E);
    idtSetEntry(12,(uint32_t)isr12, 0x08, 0x8E);
    idtSetEntry(13,(uint32_t)isr13, 0x08, 0x8E);
    idtSetEntry(14,(uint32_t)isr14, 0x08, 0x8E);
    idtSetEntry(15,(uint32_t)isr15, 0x08, 0x8E);
    idtSetEntry(16,(uint32_t)isr16, 0x08, 0x8E);
    idtSetEntry(17,(uint32_t)isr17, 0x08, 0x8E);
    idtSetEntry(18,(uint32_t)isr18, 0x08, 0x8E);
    idtSetEntry(19,(uint32_t)isr19, 0x08, 0x8E);
    idtSetEntry(20,(uint32_t)isr20, 0x08, 0x8E);
    idtSetEntry(21,(uint32_t)isr21, 0x08, 0x8E);
    idtSetEntry(22,(uint32_t)isr22, 0x08, 0x8E);
    idtSetEntry(23,(uint32_t)isr23, 0x08, 0x8E);
    idtSetEntry(24,(uint32_t)isr24, 0x08, 0x8E);
    idtSetEntry(25,(uint32_t)isr25, 0x08, 0x8E);
    idtSetEntry(26,(uint32_t)isr26, 0x08, 0x8E);
    idtSetEntry(27,(uint32_t)isr27, 0x08, 0x8E);
    idtSetEntry(28,(uint32_t)isr28, 0x08, 0x8E);
    idtSetEntry(29,(uint32_t)isr29, 0x08, 0x8E);
    idtSetEntry(30,(uint32_t)isr30, 0x08, 0x8E);
    idtSetEntry(31,(uint32_t)isr31, 0x08, 0x8E);
    
    loadIdt((uint32_t)&idtPointer);
}

// This gets called from our ASM interrupt handler stub.
void isr_handler(registers_t regs)
{
   kWriteLog("recieved interrupt: ");
   kWriteLog_format1d("%d", (uint32_t)regs.int_no);
   kWriteLog('\n');
}
