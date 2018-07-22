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
                enum ata_disk_select ata_port = (enum ata_disk_select)portnumber;
                
                for(size_t i = 0; i < dm->disks->count; i++)
                {
                    // If we find a disk that is already connected return it.
                    struct disk* d = (struct disk*)dm->disks->dataElements[i];
                    if(d->diskBus == bus && d->portNumber == portnumber)
                    {
                        result = d;
                        
                        return DM_OK;
                    }
                }
                
                driver_ata_select_drive(ata_port);
                
                BOOL res = driver_ata_valid_disk();
                
                if(res)
                {
                    struct ata_identify_device drive_info;
                    memset(&drive_info, 0, sizeof(struct ata_identify_device));
                    int identify = driver_ata_identify(&drive_info);
                    
                    // If the drive can't respond to IDENTIFY, drop it.
                    if(identify != ATA_ERROR_OK)
                    {
                        kWriteLog("[DM] Disk %d failed to IDENTIFY", ata_port);
                        
                        return DM_DISKINVALID;
                    }
                    
                    result = (struct disk*)kmalloc(sizeof(struct disk));
                    
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
    if(target->pendingOperation)
    {
        return DM_IO_PENDING;
    }
    
    if(vector_exists(target->disk_master->disks, target) == FALSE)
    {
        return DM_DISKNOTFOUND;
    }
    
    vector_remove(target->disk_master->disks, target);
    
    return DM_OK;
}

int disk_status(struct disk* target, uint32_t* status)
{
    (void)target;
    (void)status;
    
    return DM_OK;
}

int disk_read(struct disk* target, uint32_t address, uint32_t length, uint8_t** buffer, size_t* readBytes)
{
    *readBytes = 0; // Set the bytes to 0 so we can early return with 0 read.
    
    if(buffer == NULL || readBytes == NULL || target == NULL || length == 0)
        return DM_BADREQUEST;

    if(target->pendingOperation)
        return DM_IO_PENDING;
    
    if(target->diskBus == BUS_ATA)
    {
        int res = driver_ata_select_drive(target->portNumber);
        
        if(res != ATA_ERROR_OK)
            return DM_DISKINVALID;
        
        // No information about the request is transferred back to the caller.
        // TODO : Error and status check
        uint8_t* input_bytes = read_data(address, length);
        *readBytes = length;
        *buffer = input_bytes;
    }
    else
    {
        return DM_INVALID_BUS;
    }
    
    return DM_OK;
}

int disk_write(struct disk* target, uint32_t startAddress, uint8_t* buffer, size_t buflen)
{
    if(target == NULL || buffer == NULL)
        return DM_BADREQUEST;
    
    if(target->pendingOperation)
        return DM_IO_PENDING;
        
    if(target->diskBus == BUS_ATA)
    {
        int res = driver_ata_select_drive(target->portNumber);
        
        if(res != ATA_ERROR_OK)
            return DM_DISKINVALID;
        
        write_data(buffer, buflen, startAddress);
    }
    else
    {
        return DM_INVALID_BUS;
    }
    
    return DM_OK;
}
