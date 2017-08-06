#include "ezfs.h"

#include "ata_driver.h"
#include "memory.h"
#include "array_utils.h"
#include "string.h"
#include "math.h"

/* Create a file on disk.
 * @param dir - Parent directory
 * @param name - Name of the file
 * @param access - Starting file access for the file. Not used currently.
 * @param flags - File flags. Not used currently.
 *
 * @return File handle of the new file. -1 if allocation was not successful.
 */
file_h ezfs_create_file(file_h dir, char* name, enum FS_FILE_ACCESS access, enum FS_FILE_FLAGS flags)
{
    (void)dir;
    (void)access;
    (void)flags;
    
    
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

/* Get a file handle from a file name.
 * @param name - Name of the file.
 * 
 * @return File handle. FILE_NOT_FOUND if no file exists.
 */
file_h ezfs_find_file(char* name)
{
    for(uint32_t i = 0; i < loaded_metablock->files_amount; i++)
    {
        struct file_allocation* alloc = (allocated_files + i);
        
        if(strcmp(alloc->name, name) == 0 && alloc->allocated == TRUE)
        {
            return alloc->id;
        }
    }
    
    return FILE_NOT_FOUND;
}

/* Read file data.
 * @param file - Handle of the file to read.
 * @param[OUT] buf - Buffer to receie the data. Initialized by this function.
 *
 * @return Size of the buffer's data.
 */
size_t ezfs_read_file(file_h file, uint8_t** buf)
{
    struct file_allocation* found = ezfs_find_file_info(file);

    if(found == NULL)
    {
        buf = NULL;
        
        return 0;
    }
    
    uint64_t diskAddr = found->dataBlockDiskAddress;
    uint32_t diskSize = found->dataSize;
    
    *buf = read_data(diskAddr, diskSize);
    
    return diskSize;
}

/* Write data to file.
 * @param file - Handle of the file to write.
 * @param buf - Data to write.
 * @param bufLen - Length of the write buffer.
 *
 * @return Amount of bytes that was written.
 */
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

/* Rename a file.
 * @param file - Handle of the file to rename. 
 * @param toName - New name of the file.
 */
void ezfs_rename_file(file_h file, char* toName)
{
    struct file_allocation* alloc = ezfs_find_file_info(file);
    
    if(alloc != NULL)
    {
        strcpy(alloc->name, toName);
        
        ezfs_write_allocation_to_disk(alloc);
    }
}

/* Set file protection for a file.
 * @param file - File handle.
 * @param access - New file access flags.
 *
 * Function not yet implemented.
 */
void ezfs_protect_file(file_h file, enum FS_FILE_ACCESS access)
{
    (void)file;
    (void)access;
    // File permissions not implemented yet.
}

/* Delete a file.
 * @param file - Handle of file to delete.
 */
void ezfs_delete_file(file_h file)
{
    struct file_allocation* alloc = ezfs_find_file_info(file);
    
    if(alloc != NULL)
    {
        ezfs_deallocate(alloc);
    }
}

/* Initial function to call in order to work with an EZFS drive.
 * The function will load the existing metablock to check if it is formatted 
 * before loading it.
 */
void ezfs_prepare_disk()
{
    struct filesystem_metablock* ondisk_metablock = (struct filesystem_metablock*)read_data(METABLOCK_ADDRESS, sizeof(struct filesystem_metablock));
    
    allocation_pointers = NULL;
    
    if(strncmp(ondisk_metablock->magic, "ezfs", 4) == 0)
    {
        loaded_metablock = ondisk_metablock;
        ezfs_load_disk_allocation_area();
    }
    else
    {
        ezfs_format_disk();
    }
    
    ezfs_init_allocation_pointers();
}

/* Format the disk to prepare it for usage.
 * Will overwrite the metablock area and the allocation area.
 */
void ezfs_format_disk()
{
    struct filesystem_metablock* block = ezfs_create_metablock();
    
    ezfs_write_metablock(block);
    
    loaded_metablock = block;
    
    ezfs_format_allocation_area();
}

/* Create a metablock instance. The block is not slotted into the 
 * fs driver yet so multiple instances can be handled at once.
 * @return New instance of a filesystem_metablock.
 */
struct filesystem_metablock* ezfs_create_metablock()
{
    struct filesystem_metablock* block = (struct filesystem_metablock*)malloc(sizeof(struct filesystem_metablock));
    strcpy(block->magic, "ezfs");
    block->files_amount = 0;
    
    block->allocation_area_start = METABLOCK_ADDRESS + sizeof(struct filesystem_metablock);
    block->allocation_area_length = sizeof(struct file_allocation) * MAX_FILES_NUM;
    
    // TODO : Call ata_driver to get the disk size.
    struct ata_identify_device* deviceInfo = (struct ata_identify_device*)malloc(sizeof(struct ata_identify_device));
    driver_ata_identify(deviceInfo);

    block->data_area_start = block->allocation_area_start + block->allocation_area_length;
    block->data_area_length = deviceInfo->addressable_sectors_lba28 * 512;
    
    block->version_fs_major = 0;
    block->version_fs_minor = 1;
    
    free(deviceInfo);
    
    return block;
}

/* Read and load the metablock from the disk.
 * The block is not loaded yet, only created.
 */
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

/* Write a metablock to the disk.
 * @param block - Metablock to write.
 */
void ezfs_write_metablock(struct filesystem_metablock* block)
{
    write_data((uint8_t*)block, BLOCK_SIZE, METABLOCK_ADDRESS);
}

/* Create a new allocation_area, initializes it and write it to disk.
 * This will enable this allocation area in the fs driver.
 * Allows for MAX_FILES_NUM number of files.
 */
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

/* Load the allocation area from the disk and enables it in the fs driver.
 *
 */
void ezfs_load_disk_allocation_area()
{
    uint8_t* allocData = read_data(loaded_metablock->allocation_area_start, loaded_metablock->allocation_area_length);
    
    allocated_files = (struct file_allocation*)allocData;
}

/* Write the current allocation area to the disk.
 */
void ezfs_write_allocation_area()
{
    write_data((uint8_t*)allocated_files, loaded_metablock->allocation_area_start, sizeof(struct file_allocation) * MAX_FILES_NUM);
}

/* Tries to find a free area of disk space enough to contain 'size' bytes.
 * The file allocations must be ordered by disk address for this to work.
 * @param size - Minimum size that must be accommodated.
 *
 * @return Disk address where the free space was found.
 */
uint64_t ezfs_find_free_space(size_t size)
{
    ezfs_sort_allocations();
    
    for(int i = 0; i < MAX_FILES_NUM; i++)
    {
        struct file_allocation* alloc = allocation_pointers[i];
        struct file_allocation* next_alloc = allocation_pointers[i + 1];
        
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
            // Means no files have been allocated yet. Gotta start somewhere.
            return loaded_metablock->data_area_start;
        }
    }
    
    return loaded_metablock->data_area_start;
}

/* Return the free space between two allocations.
 * @param one - First file.
 * @param two - Second file.
 * @return Available space between the first and second file.
 */
size_t ezfs_get_free_space_between_files(struct file_allocation* one, struct file_allocation* two)
{
    // TODO : Check that first file comes before second file.
    return two->dataBlockDiskAddress - (one->dataBlockDiskAddress + one->diskSize);
}

/* Checks if the file can grow without overlapping on the next file.
 * @param file - File to check.
 * @param required - Amount of space required.
 * @return Whether of not the file can grow.
 */
BOOL ezfs_data_can_grow(struct file_allocation* file, size_t required)
{
    struct file_allocation* nextFile = allocation_pointers[file->fileNumber];
    
    return ezfs_get_free_space_between_files(file, nextFile) >= required;
}

/* Move the file's data to another area on disk where enough free space to grow
 * is available.
 * @param file - File to relocate.
 * @param required - Minimum amount of free space needed.
 * @return TRUE if the operation was successful.
 */
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

/* Calculate the next BLOCK_SIZE aligned size.
 * @param size - Current unaligned byte size.
 * @return Padded size.
 */
uint64_t ezfs_calculate_new_padded_size(uint64_t size)
{
    return ceil(size / BLOCK_SIZE) * BLOCK_SIZE;
}

/* Write the selected file allocation to the disk.
 * @param file - Target file.
 */
void ezfs_write_allocation_to_disk(struct file_allocation* file)
{
    uint64_t disk_alloc_address = loaded_metablock->allocation_area_start
        + (sizeof(struct file_allocation) * file->fileNumber);
        
    write_data((uint8_t*)file, sizeof(struct file_allocation), disk_alloc_address);
}

/* Returns the file_allocation object for the target handle.
 * @param file - File handle.
 * @return File allocation object. Return NULL if not found.
 */
struct file_allocation* ezfs_find_file_info(file_h file)
{
    for(int i = 0; i < MAX_FILES_NUM; i++)
    {
        struct file_allocation* alloc = (allocated_files + i);
        if(alloc->id == file)
        {
            return alloc;
        }
    }
    
    return NULL;
}

/* Remove a file allocation from the table and reset the entry.
 * @param file - Target file to remove.
 * Depending if ZERO_ON_DELETE is defined, the file data will be zeroed.
 */
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

void ezfs_sort_allocations()
{
    struct file_allocation** newList = (struct file_allocation**)malloc(sizeof(struct file_allocation*) * MAX_FILES_NUM);
    
    for(int i = 0; i < MAX_FILES_NUM; i++)
    {
        struct file_allocation* current = allocated_files + i;
        
        for(int k = i + 1; k < MAX_FILES_NUM; k++)
        {
            struct file_allocation* next = allocated_files + k;
            
            if(next->allocated == TRUE && next->dataBlockDiskAddress < current->dataBlockDiskAddress)
            {
                struct file_allocation* temp = current;
                current = next;
                next = temp;
            }
        }
        
        newList[i] = current;
    }
    
    free(allocation_pointers);
    
    allocation_pointers = newList;
}

void ezfs_init_allocation_pointers()
{
    for(int i = 0; i < MAX_FILES_NUM; i++)
    {
        struct file_allocation* ptr = allocated_files + i;
        
        allocation_pointers[i] = ptr;
    }
}

/* Writes 0's to a file's disk area.
 * @param file - Target file.
 */
void ezfs_zero_file(struct file_allocation* file)
{
    uint8_t* zeros = malloc(file->diskSize);
    array_set(zeros, 0, file->diskSize);
    size_t zeroCount = ezfs_write_file(file->id, zeros, file->diskSize);
    
    free(zeros);
    ASSERT(zeroCount == file->diskSize, "ezfs_zero_file wrong length.");
}

/* Copy a file's data from one location to another.
 * @param file - Target file.
 * @param diskAddr - New address.
 */
void ezfs_copy_data(struct file_allocation* file, uint64_t diskAddr)
{
    uint8_t* currentData = read_data(file->dataBlockDiskAddress, file->dataSize);
    
    if(currentData != NULL)
    {
        write_data(currentData, file->dataSize, diskAddr);
        
        free(currentData);
    }
}

