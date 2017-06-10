#include "ata_monitor.h"

#include "io_func.h"
#include "framebuffer.h"

// Temporary, need to get the port from PCI info
#define DRIVE_PORT 0x1F0//0xC040

// Registers offset
#define DATA_PORT 0x0
#define FEAT_ERRO 0x1
#define SEC_COUNT 0x2
#define SEC_NUM 0x3
#define CYL_LOW 0x4
#define CYL_HIGH 0x5
#define DRIVE_HEAD 0x6
#define COMMAND_REG_STATUS 0x7

// Status flags
#define STATUS_ERR 0x1
#define STATUS_DRQ 0x8
#define STATUS_SRV 0x10
#define STATUS_DF  0x20
#define STATUS_RDY 0x40
#define STATUS_BSY 0x80


// Send 0xE0 for the "master" or 0xF0 for the "slave", ORed with the highest 4 bits of the LBA to port 0x1F6: outb(0x1F6, 0xE0 | (slavebit << 4) | ((LBA >> 24) & 0x0F))
// Send a NULL byte to port 0x1F1, if you like (it is ignored and wastes lots of CPU time): outb(0x1F1, 0x00)
// Send the sectorcount to port 0x1F2: outb(0x1F2, (unsigned char) count)
// Send the low 8 bits of the LBA to port 0x1F3: outb(0x1F3, (unsigned char) LBA))
// Send the next 8 bits of the LBA to port 0x1F4: outb(0x1F4, (unsigned char)(LBA >> 8))
// Send the next 8 bits of the LBA to port 0x1F5: outb(0x1F5, (unsigned char)(LBA >> 16))
// Send the "READ SECTORS" command (0x20) to port 0x1F7: outb(0x1F7, 0x20)
// Wait for an IRQ or poll.
// Transfer 256 16-bit values, a uint16_t at a time, into your buffer from I/O port 0x1F0. (In assembler, REP INSW works well for this.)
// Then loop back to waiting for the next IRQ (or poll again -- see next note) for each successive sector.

uint32_t cycleWaiting;

void test_io_port()
{
    cycleWaiting = 0;
    
    //Debugger();
    
    unsigned char res = get_status();
        
    outb(DRIVE_PORT + DATA_PORT, 0x47);
    outb(DRIVE_PORT + DRIVE_HEAD, 0xE0);
    outb(DRIVE_PORT + FEAT_ERRO, 0x0);
    outb(DRIVE_PORT + SEC_COUNT, 1);
    outb(DRIVE_PORT + SEC_NUM, 0);
    outb(DRIVE_PORT + CYL_LOW, 0);
    outb(DRIVE_PORT + CYL_HIGH, 0);
    outb(DRIVE_PORT + COMMAND_REG_STATUS, 0x30);
    outb(DRIVE_PORT + COMMAND_REG_STATUS, 0x30);
    
    res = get_status();
    
    Debugger();    
    
    for(int i = 0; i < 257; i++)
    {
        outw(DRIVE_PORT + DATA_PORT, (0x47 | (0x48 << 8)));
        outb(DRIVE_PORT + COMMAND_REG_STATUS, 0xE7);
    }
    
    wait_for_ready();
    
    outb(DRIVE_PORT + DRIVE_HEAD, 0xE0);
    outb(DRIVE_PORT + FEAT_ERRO, 0x0);
    outb(DRIVE_PORT + SEC_COUNT, 1);
    outb(DRIVE_PORT + SEC_NUM, 0);
    outb(DRIVE_PORT + CYL_LOW, 0);
    outb(DRIVE_PORT + CYL_HIGH, 0);
    outb(DRIVE_PORT + COMMAND_REG_STATUS, 0x20);
    
    uint16_t buf[257];
    
    for(int j = 0; j < 257; j++)
        buf[j] = 0;
    
    for(int k = 0; k < 257; k++)
        buf[k] = inw(DRIVE_PORT + DATA_PORT);
    
    wait_for_ready();
    
    fbMoveCursor(0, 0);
    fbPutString("Done ?");
    
    Debugger();
}

void read_bytes(int count, unsigned char* buffer)
{
    unsigned short port = 0xC040;
    
    for(int i = 0; i < count; i++)
    {
        buffer[i] = inb(port);
    }
}

void wait_for_ready()
{
    while(TRUE)
    {
        unsigned char status = get_status();
        
        if((status & STATUS_BSY) == 0 && (status & STATUS_DRQ) != 0)
        {
            return;
        }
        else if((status & STATUS_ERR) != 0 && (status & STATUS_DF) != 0)
        {
            ASSERT(FALSE, "ERROR OR DRIVE FAULT");
            
            return;
        }
        
        cycleWaiting++;
    }
}

unsigned char get_status()
{
    unsigned char res = inb(DRIVE_PORT + COMMAND_REG_STATUS);
    res = inb(DRIVE_PORT + COMMAND_REG_STATUS);
    res = inb(DRIVE_PORT + COMMAND_REG_STATUS);
    res = inb(DRIVE_PORT + COMMAND_REG_STATUS);
    res = inb(DRIVE_PORT + COMMAND_REG_STATUS);

    return res;
}

void io_wait()
{
    while(TRUE)
    {
    }
}
