#include "filesystem.h"

#include "ata_driver.h"
#include "math.h"
#include "memory.h"
#include "array_utils.h"
#include "ezfs.h"

/** 
 * Prepares the disk for IO. This selects the first disk on the master bus.
 * Will set the FilesystemReady property of the filesytem struct to TRUE
 * if the setup was successful.
 */
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
