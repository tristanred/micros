#include "ata_driver.h"


#include "io_func.h"
#include "framebuffer.h"
#include "kernel.h"
#include "memory.h"
#include "string.h"

void init_module_ata_driver(struct kernel_info_block* kinfo)
{
    ata_driver = alloc_kernel_module(sizeof(struct ata_driver_info));
    kinfo->m_ata_driver = ata_driver;
    
    ata_driver->currentDisk = ATA_NONE;
    ata_driver->driverError = FALSE;
}

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
    Debugger();
    
    driver_ata_select_drive(ATA_MASTER_0);
    
    uint16_t* data = malloc(sizeof(uint16_t) * 256);
    for(int i = 0; i < 256; i++)
    {
        data[i] = i;
    }
    driver_ata_write_sectors(data, 1, 4);
    driver_ata_flush_cache();
    
    uint16_t* dataBack = driver_ata_read_sectors(1, 4);
    
    for(int k = 0; k < 256; k++)
    {
        if(data[k] != dataBack[k])
        {
            ASSERT(FALSE, "DATA INCORRECT");
        }
    }
}

unsigned char get_status()
{
    unsigned char res = inb(ata_driver->currentDiskPort + COMMAND_REG_STATUS);
    res = inb(ata_driver->currentDiskPort + COMMAND_REG_STATUS);
    res = inb(ata_driver->currentDiskPort + COMMAND_REG_STATUS);
    res = inb(ata_driver->currentDiskPort + COMMAND_REG_STATUS);
    res = inb(ata_driver->currentDiskPort + COMMAND_REG_STATUS);

    return res;
}

enum ata_driver_status driver_ata_get_status()
{
    unsigned char result = get_status();
    
    if((result & STATUS_READY) && (result & STATUS_BUSY) == 0)
    {
        return ATA_OK;
    }
    else
    {
        return ATA_ERROR;
    }
}

void driver_ata_wait_for_clear_bit(unsigned char statusBits)
{
    uint64_t _debug_loopCount = 0;
    
    while(TRUE)
    {
        unsigned char status = get_status();
        
        if((status & statusBits) == 0)
        {
            return;
        }
        
        #ifdef REPORT_STUCK_IO
        
        _debug_loopCount++;
        
        if(_debug_loopCount > STUCK_IO_LOOP_TRESH)
        {
            char msg[256];
            
            sprintf_1d(msg, "IO STUCK WAITING FOR STATUS %d TO CLEAR.", statusBits);
            
            fbPutString(msg);
        }
        
        #endif
    }
}

void driver_ata_wait_for_set_bit(unsigned char statusBits)
{
    uint64_t _debug_loopCount = 0;
    
    while(TRUE)
    {
        unsigned char status = get_status();
        
        if((statusBits & status) == statusBits)
        {
            return;
        }
        
        #ifdef REPORT_STUCK_IO
        
        _debug_loopCount++;
        
        if(_debug_loopCount > STUCK_IO_LOOP_TRESH)
        {
            char msg[256];
            
            sprintf_1d(msg, "IO STUCK WAITING FOR STATUS %d TO SET.", statusBits);
            
            fbPutString(msg);
        }
        
        #endif
    }
}

void driver_ata_wait_for_only_set_bit(unsigned char statusBits)
{
    uint64_t _debug_loopCount = 0;
    
    // TODO
    while(TRUE)
    {
        unsigned char status = get_status();
        
        if((statusBits & status) == statusBits)
        {
            return;
        }
        
        #ifdef REPORT_STUCK_IO
        
        _debug_loopCount++;
        
        if(_debug_loopCount > STUCK_IO_LOOP_TRESH)
        {
            char msg[256];
            
            sprintf_1d(msg, "IO STUCK WAITING FOR STATUS %d TO ONLY SET.", statusBits);
            
            fbPutString(msg);
        }
        
        #endif

    }
}

void driver_ata_resetdisk()
{
    
}

void driver_ata_select_drive(enum ata_disk_select disk)
{
    switch(disk)
    {
        case ATA_MASTER_0:
        {
            outb(PRIMARY_PORT + DRIVE_HEAD, MASTER_PORT_SELECTOR);
            ata_driver->currentDiskPort = PRIMARY_PORT;
            break;
        };
        case ATA_SLAVE_0:
        {
            outb(PRIMARY_PORT + DRIVE_HEAD, SLAVE_PORT_SELECTOR);
            ata_driver->currentDiskPort = PRIMARY_PORT;            
            break;
        };
        case ATA_MASTER_1:
        {
            outb(SECONDARY_PORT + DRIVE_HEAD, MASTER_PORT_SELECTOR);
            ata_driver->currentDiskPort = SECONDARY_PORT;
            break;
        };
        case ATA_SLAVE_1:
        {
            outb(SECONDARY_PORT + DRIVE_HEAD, SLAVE_PORT_SELECTOR);
            ata_driver->currentDiskPort = SECONDARY_PORT;
            break;
        };
        default:
        {
            ASSERT(FALSE, "UNKNOWN DISK SELECT");
            
            break;
        }
    }
    
    driver_ata_wait_for_set_bit(STATUS_READY);
    driver_ata_wait_for_clear_bit(STATUS_BUSY);
    
    ata_driver->currentDisk = disk;
}

void driver_ata_select_drive_with_lba_bits(enum ata_disk_select disk, uint8_t top_lba_bits)
{
    switch(disk)
    {
        case ATA_MASTER_0:
        {
            outb(PRIMARY_PORT + DRIVE_HEAD, MASTER_PORT_SELECTOR | top_lba_bits);
            ata_driver->currentDiskPort = PRIMARY_PORT;
            break;
        };
        case ATA_SLAVE_0:
        {
            outb(PRIMARY_PORT + DRIVE_HEAD, SLAVE_PORT_SELECTOR | top_lba_bits);
            ata_driver->currentDiskPort = PRIMARY_PORT;            
            break;
        };
        case ATA_MASTER_1:
        {
            outb(SECONDARY_PORT + DRIVE_HEAD, MASTER_PORT_SELECTOR | top_lba_bits);
            ata_driver->currentDiskPort = SECONDARY_PORT;
            break;
        };
        case ATA_SLAVE_1:
        {
            outb(SECONDARY_PORT + DRIVE_HEAD, SLAVE_PORT_SELECTOR | top_lba_bits);
            ata_driver->currentDiskPort = SECONDARY_PORT;
            break;
        };
        default:
        {
            ASSERT(FALSE, "UNKNOWN DISK SELECT");
            
            break;
        }
    }
    
    driver_ata_wait_for_set_bit(STATUS_READY);
    driver_ata_wait_for_clear_bit(STATUS_BUSY);
    
    ata_driver->currentDisk = disk;
}

void driver_ata_identify(struct ata_identify_device* drive_info)
{
    if(ata_driver->currentDisk == ATA_NONE)
    {
        ata_driver->driverError = TRUE;
        return;
    }

    // Send the command
    outb(ata_driver->currentDiskPort + COMMAND_REG_STATUS, 0xEC);
    
    driver_ata_wait_for_set_bit(STATUS_DATA_REQUEST);
    
    // Read 512 bytes from the device
    uint16_t buf[256];
    
    for(int i = 0; i < 256; i++)
    {
        buf[i] = 0;
        buf[i] = inw(ata_driver->currentDiskPort+ DATA_PORT);
    }
    
    memcpy(drive_info, buf, sizeof(struct ata_identify_device));
}

void driver_ata_flush_cache()
{
    outb(ata_driver->currentDiskPort + COMMAND_REG_STATUS, 0xE7);
    
    driver_ata_wait_for_clear_bit(STATUS_BUSY);
}

uint16_t* driver_ata_read_sectors(uint8_t sectorCount, uint64_t startingSector)
{
    unsigned char res = get_status();
    
    driver_ata_select_drive_with_lba_bits(ata_driver->currentDisk, ((startingSector >> 24) & 0xF));
    
    uint16_t* buf = malloc(sizeof(uint16_t) * 256);
    
    outb(ata_driver->currentDiskPort + FEAT_ERRO, 0x0);
    outb(ata_driver->currentDiskPort + SEC_COUNT, sectorCount);
    outb(ata_driver->currentDiskPort + LBA_LOW, (startingSector) & 0xF);
    outb(ata_driver->currentDiskPort + LBA_MID, (startingSector >> 8) & 0xF);
    outb(ata_driver->currentDiskPort + LBA_HIGH, (startingSector >> 16) & 0xF);
    outb(ata_driver->currentDiskPort + COMMAND_REG_STATUS, 0x20);
    
    driver_ata_wait_for_set_bit(STATUS_DATA_REQUEST);
    
    int readCount = sectorCount * 256;
    if(sectorCount == 0)
        readCount = 256 * 256;
        
    for(int i = 0; i < readCount; i++)
    {
        buf[i] = 0;
        buf[i] = inw(ata_driver->currentDiskPort + DATA_PORT);
        
        driver_ata_wait_for_clear_bit(STATUS_BUSY);
        
        res = get_status();
    }
    
    res = get_status();
    
    if((res & STATUS_DATA_REQUEST) == STATUS_DATA_REQUEST || (res & STATUS_ERROR) == STATUS_ERROR)
    {
        ASSERT(FALSE, "ATA READ STILL PENDING.");
    }
    
    return buf;
}

void driver_ata_write_sectors(uint16_t* data, uint8_t sectorCount, uint64_t startingSector)
{
    unsigned char res = get_status();
    
    driver_ata_select_drive_with_lba_bits(ata_driver->currentDisk, ((startingSector >> 24) & 0xF));
    
    outb(ata_driver->currentDiskPort + FEAT_ERRO, 0x0);
    outb(ata_driver->currentDiskPort + SEC_COUNT, sectorCount);
    outb(ata_driver->currentDiskPort + LBA_LOW, (startingSector) & 0xF);
    outb(ata_driver->currentDiskPort + LBA_MID, (startingSector >> 8) & 0xF);
    outb(ata_driver->currentDiskPort + LBA_HIGH, (startingSector >> 16) & 0xF);
    outb(ata_driver->currentDiskPort + COMMAND_REG_STATUS, 0x30);
    
    driver_ata_wait_for_set_bit(STATUS_DATA_REQUEST);
    
    // This is the code for a 'write bytes until DRQ is off' type of write.
    // int i = 0;
    // while(TRUE)
    // {
    //     res  = get_status();
        
    //     driver_ata_wait_for_clear_bit(STATUS_BUSY);
        
    //     outw(ata_driver->currentDiskPort + DATA_PORT, data[i]);
        
    //     res  = get_status();
        
    //     driver_ata_wait_for_clear_bit(STATUS_BUSY);
        
    //     if((res & STATUS_DATA_REQUEST) == 0)
    //     {
    //         break;
    //     }
        
    //     i++;
    // }
    
    int writeCount = sectorCount * 256;
    if(sectorCount == 0)
        writeCount = 256 * 256;
    
    for(int i = 0; i < writeCount; i++)
    {
        outw(ata_driver->currentDiskPort + DATA_PORT, data[i]);
        
        // Wasting a bit of time here since the disk only seems to get busy every
        // 255 writes so he can write his buffer.
        driver_ata_wait_for_clear_bit(STATUS_BUSY);
        
        res = get_status();
    }
    
    res = get_status();
    
    if((res & STATUS_DATA_REQUEST) == STATUS_DATA_REQUEST || (res & STATUS_ERROR) == STATUS_ERROR)
    {
        ASSERT(FALSE, "ATA WRITE STILL PENDING.");
    }
}

void driver_ata_write_test_sectors()
{
    unsigned char res = get_status();
    
    uint16_t* dat = malloc(sizeof(uint16_t) * (128*512));
    
    for(int i = 0; i < 128*512; i++)
    {
        dat[i] = 0xF2F2;
    }
    
    driver_ata_write_sectors(dat, 0, 0);
    res = get_status();
    
    driver_ata_write_sectors(dat, 0, 1);
    res = get_status();
    
    driver_ata_write_sectors(dat, 0, 2);
    res = get_status();
    
    driver_ata_write_sectors(dat, 0, 3);
    res = get_status();
    
    driver_ata_write_sectors(dat, 0, 4);
    res = get_status();
    
    driver_ata_write_sectors(dat, 0, 5);
    res = get_status();
    
    driver_ata_write_sectors(dat, 0, 6);
    res = get_status();
    
    if((res & STATUS_DATA_REQUEST) == STATUS_DATA_REQUEST || (res & STATUS_ERROR) == STATUS_ERROR)
    {
        ASSERT(FALSE, "TEST SECTORS FAILED.");
    }
}
