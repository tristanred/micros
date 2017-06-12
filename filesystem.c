#include "filesystem.h"

#include "ata_driver.h"

void setup_filesystem()
{
    driver_ata_select_drive(ATA_MASTER_0);
    
    enum ata_driver_status res = driver_ata_get_status();
    
    if(res != ATA_OK)
    {
        // Disk unavailable
        filesystem->FilesystemReady = FALSE;
    }
    else
    {
        filesystem->FilesystemReady = TRUE;
    }
}

uint8_t* read_data(uint64_t startAddress, uint64_t length)
{
    uint8_t offset = 0;
    uint64_t sectorTarget = get_sector_from_address(startAddress, &offset);
    uint64_t neededSectors = length / 512;
    
    if(neededSectors > 255)
    {
        // TODO Need to do multiple reads
    }
    
    uint8_t neededSectors_b = (uint8_t)neededSectors & 0xFF;
    
    uint8_t* data = (uint8_t*)driver_ata_read_sectors(neededSectors_b, sectorTarget);
    
    return data;
}

void write_data(uint8_t* data, uint64_t length, uint64_t startAddress)
{
    uint8_t offset = 0;
    uint64_t sectorTarget = get_sector_from_address(startAddress, &offset);
    uint64_t neededSectors = length / 512;
    
    if(neededSectors > 255)
    {
        // TODO Need to do multiple reads
    }
    
    uint8_t neededSectors_b = (uint8_t)neededSectors & 0xFF;
    
    driver_ata_write_sectors((uint16_t*)data, neededSectors_b, sectorTarget);
}

uint64_t get_sector_from_address(uint64_t address, uint8_t* sectorOffset)
{
    uint64_t lba = address & 0xFFFFFFFFFFFFFF00;
    uint8_t offset = address & 0xFF;
    
    *sectorOffset = offset;
    
    return lba;
}

