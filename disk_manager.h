#ifndef DISKMAN_H
#define DISKMAN_H

#include "common.h"
#include "vector.h"

struct ata_driver_info;

enum dm_errorcodes
{
    DM_OK = 0,
    DM_BADREQUEST,
    DM_DISKNOTFOUND,
    DM_DISKINVALID,
    DM_INVALID_DISKNB,
    DM_INVALID_BUS,
    DM_IO_PENDING,
    DM_ERRORUNKNOWN
};

enum dm_bus
{
    BUS_ATA
};

struct disk
{
    struct diskman* disk_master;
    
    enum dm_bus diskBus;
    uint32_t portNumber;
    
    BOOL pendingOperation;
    
    uint32_t identifier;
    BOOL formatted;
    
    uint32_t size;
};

struct diskman
{
    struct vector* disks;
    struct ata_driver_info* ata_driver_ref;
};

/**
 * Create a Disk manager instance. This object is responsible for overseeing
 * the operations to and from the computer's disks. One reason is because 
 * operations have to be serialized, they have to check if the disk is ready
 * before doing ops, 
 */
struct diskman* create_diskman();

int connect_disk(struct diskman* dm, enum dm_bus bus, uint32_t portnumber, struct disk* result);

int disconnect_disk(struct disk* target);

int disk_status(struct disk* target, uint32_t* status);

// Buffer is initialized by the function NOT by the caller. Should be NULL on calling.
// 'readBytes' is initialized by the caller and modified by the function.
int disk_read(struct disk* target, uint32_t address, uint32_t length, uint8_t** buffer, size_t* readBytes);

int disk_write(struct disk* target, uint32_t startAddress, uint8_t* buffer, size_t buflen);

#endif
