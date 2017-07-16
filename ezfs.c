#include "ezfs.h"

#include "ata_driver.h"
#include "memory.h"
#include "array_utils.h"
#include "string.h"
#include "math.h"

file_h ezfs_create_file(file_h dir, char* name, enum FS_FILE_ACCESS access, enum FS_FILE_FLAGS flags)
{
    uint32_t allocationIndex = loaded_metablock->files_amount;
    uint64_t locatedAddress = ezfs_find_free_space(DEFAULT_FILE_SIZE);
    
    struct file_allocation* alloc = &allocated_files[allocationIndex];
    alloc->allocated = TRUE;
    strcpy(alloc->name, name);
    alloc->fileNumber = allocationIndex;
    alloc->id = allocationIndex + FS_COUNT_SPECIAL_FILES;
    alloc->dataBlockDiskAddress = locatedAddress;
    alloc->dataSize = 0;
    alloc->diskSize = DEFAULT_FILE_SIZE;
    alloc->type = 0;
    
    ezfs_write_allocation_to_disk(alloc);
    
    loaded_metablock->files_amount++;
    ezfs_write_metablock(loaded_metablock);
    
    return alloc->id;
}

file_h ezfs_find_file(char* name)
{
    for(int i = 0; i < MAX_FILES_NUM; i++)
    {
        struct file_allocation* alloc = (allocated_files + i);
        
        if(strcmp(alloc->name, name) == 0)
        {
            return alloc->id;
        }
    }
    
    return FILE_NOT_FOUND;
}

size_t ezfs_read_file(file_h file, uint8_t** buf)
{
    struct file_allocation* found = ezfs_find_file_info(file);

    if(found == FALSE)
    {
        buf = NULL;
        
        return 0;
    }
    
    uint64_t diskAddr = found->dataBlockDiskAddress;
    uint32_t diskSize = found->dataSize;
    
    *buf = read_data(diskAddr, diskSize);
    
    return diskSize;
}

size_t ezfs_write_file(file_h file, uint8_t* buf, size_t bufLen)
{
    struct file_allocation* found = ezfs_find_file_info(file);
    
    if(found == NULL)
        return 0;
    
    if(found->diskSize < bufLen)
    {
        if(ezfs_data_can_grow(found, bufLen) == FALSE)
        {
            BOOL ok = ezfs_data_relocate(found, bufLen);
            
            if(ok == FALSE)
                return 0;
        }
    }
    
    write_data(buf, bufLen, found->dataBlockDiskAddress);
    
    found->dataSize = bufLen;
    
    ezfs_write_allocation_to_disk(found);
    
    return bufLen;
}

void ezfs_rename_file(file_h file, char* toName)
{
    struct file_allocation* alloc = ezfs_find_file_info(file);
    
    if(alloc != NULL)
    {
        strcpy(alloc->name, toName);
        
        ezfs_write_allocation_to_disk(alloc);
    }
}

void ezfs_protect_file(file_h file, enum FS_FILE_ACCESS access)
{
    (void)file;
    (void)access;
    // File permissions not implemented yet.
}

void ezfs_delete_file(file_h file)
{
    struct file_allocation* alloc = ezfs_find_file_info(file);
    
    if(alloc != NULL)
    {
        ezfs_deallocate(alloc);
    }
}

/**
 * Initial function to call in order to work with an EZFS drive.
 * The function will load the existing metablock to check if it is formatted 
 * before loading it.
 */
void ezfs_prepare_disk()
{
    struct filesystem_metablock* ondisk_metablock = (struct filesystem_metablock*)read_data(METABLOCK_ADDRESS, sizeof(struct filesystem_metablock));
    
    if(strncmp(ondisk_metablock->magic, "ezfs", 4) == 0)
    {
        loaded_metablock = ondisk_metablock;
        ezfs_load_disk_allocation_area();
    }
    else
    {
        ezfs_format_disk();
    }
}

void ezfs_format_disk()
{
    struct filesystem_metablock* block = ezfs_create_metablock();
    
    ezfs_write_metablock(block);
    
    loaded_metablock = block;
    
    ezfs_format_allocation_area();
}

struct filesystem_metablock* ezfs_create_metablock()
{
    struct filesystem_metablock* block = malloc(sizeof(struct filesystem_metablock));
    strcpy(block->magic, "ezfs");
    block->files_amount = 0;
    
    block->allocation_area_start = METABLOCK_ADDRESS + sizeof(struct filesystem_metablock);
    block->allocation_area_length = sizeof(struct file_allocation) * MAX_FILES_NUM;
    
    // TODO : Call ata_driver to get the disk size.
    struct ata_identify_device* deviceInfo = malloc(sizeof(struct ata_identify_device));
    driver_ata_identify(deviceInfo);

    block->data_area_start = block->allocation_area_start + block->allocation_area_length;
    block->data_area_length = deviceInfo->addressable_sectors_lba28 * 512;
    
    block->version_fs_major = 0;
    block->version_fs_minor = 1;
    
    free(deviceInfo);
    
    return block;
}

struct filesystem_metablock* ezfs_load_disk_metablock()
{
    uint8_t* blockData = read_data(METABLOCK_ADDRESS, BLOCK_SIZE);
    
    struct filesystem_metablock* fsblock = (struct filesystem_metablock*)blockData;
    
    if(strcmp(fsblock->magic, "ezfs") == 0)
    {
        return fsblock;
    }
    
    free(blockData);
    
    return NULL;
}

void ezfs_write_metablock(struct filesystem_metablock* block)
{
    write_data((uint8_t*)block, BLOCK_SIZE, METABLOCK_ADDRESS);
}

void ezfs_format_allocation_area()
{
    size_t areaLength = sizeof(struct file_allocation) * MAX_FILES_NUM;
    // TODO : Do a disk write pattern with a single value
    void* allocArea = malloc(areaLength);
    
    array_zero((uint8_t*)allocArea, areaLength);

    allocated_files = (struct file_allocation*)allocArea;
    
    for(int i = 0; i < MAX_FILES_NUM; i++)
    {
        (allocated_files + i)->allocated = FALSE;
    }
    
    write_data((uint8_t*)allocArea, areaLength, loaded_metablock->allocation_area_start);
}

void ezfs_load_disk_allocation_area()
{
    uint8_t* allocData = read_data(loaded_metablock->allocation_area_start, loaded_metablock->allocation_area_length);
    
    allocated_files = (struct file_allocation*)allocData;
}

void ezfs_write_allocation_area()
{
    write_data((uint8_t*)allocated_files, loaded_metablock->allocation_area_start, sizeof(struct file_allocation) * MAX_FILES_NUM);
}

uint64_t ezfs_find_free_space(size_t size)
{
    for(int i = 0; i < MAX_FILES_NUM; i++)
    {
        struct file_allocation* alloc = (allocated_files + i);
        struct file_allocation* next_alloc = (allocated_files + i + 1);
        
        if(alloc->allocated == TRUE && next_alloc->allocated == TRUE)
        {
            if(ezfs_get_free_space_between_files(alloc, next_alloc) >= size)
            {
                return alloc->dataBlockDiskAddress + alloc->diskSize;
            }
        }
        else if(alloc->allocated == TRUE && next_alloc->allocated == FALSE)
        {
            // TODO : Check if end of disk, CanAllocate(size)
            return alloc->dataBlockDiskAddress + alloc->diskSize;
        }
        else
        {
            return loaded_metablock->data_area_start;
        }
    }
    
    return loaded_metablock->data_area_start;
}

size_t ezfs_get_free_space_between_files(struct file_allocation* one, struct file_allocation* two)
{
    return two->dataBlockDiskAddress - (one->dataBlockDiskAddress + one->diskSize);
}

BOOL ezfs_data_can_grow(struct file_allocation* file, size_t required)
{
    struct file_allocation* nextFile = allocated_files + file->fileNumber;
    
    return ezfs_get_free_space_between_files(file, nextFile) >= required;
}

BOOL ezfs_data_relocate(struct file_allocation* file, size_t required)
{
    // TODO : Need to keep the allocated_files ordered by start address.
    // TODO : Implement a Compact() algorithm ?
    uint64_t oldAddress = file->dataBlockDiskAddress;
    uint64_t oldSize = file->dataSize;
    
    uint64_t newSizePaddedToBlockSize = ezfs_calculate_new_padded_size(required);
    uint64_t newAddress = ezfs_find_free_space(newSizePaddedToBlockSize);
    // TODO : Check if free space is available
    ezfs_copy_data(file, newAddress);
    
    file->dataBlockDiskAddress = newAddress;
    file->diskSize = newSizePaddedToBlockSize;
    
    ezfs_write_allocation_to_disk(file);
    
    #ifdef ZERO_ON_RELOCATE
    uint8_t* zeros = malloc(oldSize);
    write_data(zeros, oldSize, oldAddress); 
    
    free(zeros);
    #endif
    
    return TRUE;
}

uint64_t ezfs_calculate_new_padded_size(uint64_t minSize)
{
    return ceil(minSize / BLOCK_SIZE) * BLOCK_SIZE;
}

void ezfs_write_allocation_to_disk(struct file_allocation* file)
{
    uint64_t disk_alloc_address = loaded_metablock->allocation_area_start
        + (sizeof(struct file_allocation) * file->fileNumber);
        
    write_data((uint8_t*)file, sizeof(struct file_allocation), disk_alloc_address);
}

struct file_allocation* ezfs_find_file_info(file_h file)
{
    for(int i = 0; i < MAX_FILES_NUM; i++)
    {
        if((allocated_files + i)->id == file)
        {
            return (allocated_files + i);
        }
    }
    
    return NULL;
}

void ezfs_deallocate(struct file_allocation* file)
{
    #ifdef ZERO_ON_DELETE
    ezfs_zero_file(file);
    #endif
    
    file->allocated = FALSE;
    array_set((uint8_t*)&file->name, 0, MAX_FILE_NAME);
    file->dataBlockDiskAddress = 0;
    file->fileNumber = 0;
    file->id = 0;
    file->dataSize = 0;
    file->diskSize = 0;
    file->type = 0;
    
    ezfs_write_allocation_to_disk(file);
    
    loaded_metablock->files_amount--;
    
    ezfs_write_metablock(loaded_metablock);
}

void ezfs_zero_file(struct file_allocation* file)
{
    uint8_t* zeros = malloc(file->diskSize);
    array_set(zeros, 0, file->diskSize);
    size_t zeroCount = ezfs_write_file(file->id, zeros, file->diskSize);
    
    free(zeros);
    ASSERT(zeroCount == file->diskSize, "ezfs_zero_file wrong length.");
}

void ezfs_copy_data(struct file_allocation* file, uint64_t diskAddr)
{
    uint8_t* currentData = read_data(file->dataBlockDiskAddress, file->dataSize);
    
    if(currentData != NULL)
    {
        write_data(currentData, file->dataSize, diskAddr);
        
        free(currentData);
    }
}

