#include "disk_manager.h"

#include "ata_driver.h"

struct diskman* create_diskman()
{
    struct diskman* data = (struct diskman*)kmalloc(sizeof(struct diskman));
    
    data->disks = vector_create();
    
    if(kernel_info->m_ata_driver == 0)
    {
        // ATA driver not yet initialized.
    }
    else
    {
        data->ata_driver_ref = kernel_info->m_ata_driver;
    }
    
    return data;
}

int connect_disk(struct diskman* dm, enum dm_bus bus, uint32_t portnumber, struct disk* result)
{
    switch(bus)
    {
        case BUS_ATA:
        {
            if (portnumber < 4)
            {
                enum ata_disk_select ata_port = (enum ata_disk_select)(portnumber + 1);
                
                driver_ata_select_drive(ata_port);
                
                BOOL res = driver_ata_valid_disk();
                
                if(res)
                {
                    result = (struct disk*)kmalloc(sizeof(struct disk));
                    
                    struct ata_identify_device drive_info;
                    memset(&drive_info, 0, sizeof(struct ata_identify_device));
                    int identify = driver_ata_identify(&drive_info);
                    
                    // If the drive can't respond to IDENTIFY, drop it.
                    if(identify != ATA_ERROR_OK)
                    {
                        return DM_DISKINVALID;
                    }
                    
                    result->size = 0;//drive_info.user_addressable_sectors_count * 512;
                    result->disk_master = dm;
                    
                    vector_add(dm->disks, result);
                    
                    return DM_OK;
                }
                else
                {
                    return DM_DISKINVALID;
                }
            }
            else
            {
                return DM_INVALID_DISKNB;
            }
            
            break;
        }
        default:
        {
            return DM_INVALID_BUS;
        }
    }
    
    return DM_ERRORUNKNOWN;
}

int disconnect_disk(struct disk* target)
{
    (void)target;
    
    return DM_OK;
}

int disk_status(struct disk* target, uint32_t* status)
{
    (void)target;
    (void)status;
    
    return DM_OK;
}

int disk_read(struct disk* target, uint32_t address, uint32_t length, uint8_t* buffer, size_t* readBytes)
{
    (void)target;
    (void)address;
    (void)length;
    (void)buffer;
    (void)readBytes;
    
    return DM_OK;
}

int disk_write(struct disk* target, uint32_t startAddress, uint8_t* buffer, size_t buflen)
{
    (void)target;
    (void)startAddress;
    (void)buffer;
    (void)buflen;
    
    return DM_OK;
}
