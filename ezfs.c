#include "ezfs.h"

#include "ata_driver.h"
#include "memory.h"
#include "array_utils.h"
#include "string.h"

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
    
    return alloc->id;
}

size_t ezfs_read_file(file_h file, uint8_t* buf)
{
    (void)buf;
    
    struct file_allocation* found = ezfs_find_file_info(file);

    if(found == FALSE)
    {
        buf = NULL;
        
        return 0;
    }
    
    uint64_t diskAddr = found->dataBlockDiskAddress;
    uint32_t diskSize = found->dataSize;
    
    buf = read_data(diskAddr, diskSize);
    
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
    
    return bufLen;
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
    
    block->data_area_start = block->allocation_area_start + block->allocation_area_length;
    block->data_area_length = UINT64_MAX;
    
    block->version_fs_major = 0;
    block->version_fs_minor = 1;
    
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
    (void)file;
    (void)required;
    return FALSE;
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


// void format_disk()
// {
//     uint64_t start_superblock_address = boot_record_length;
//     uint8_t blankData[super_block_length];
//     array_set(blankData, 0, super_block_length);
    
//     write_data(blankData, super_block_length, start_superblock_address);
    
//     struct super_block* superblock = malloc(sizeof(struct super_block));
//     sprintf_1d(superblock->format_magic, "ext44\0", 0);
//     superblock->inodesEntriesAmount = 0;
//     superblock->inodes_map_disk_addr = start_superblock_address + super_block_length;
//     superblock->inodes_data_area_start = superblock->inodes_map_disk_addr + (sizeof(struct inode_map_entry) * MAX_FILES_NUM);
    
//     memcpy(blankData, (uint8_t*)superblock, sizeof(struct super_block));
    
//     write_data(blankData, super_block_length, start_superblock_address);
    
//     disk_super_block = superblock;
// }

// void load_super_block()
// {
//     struct super_block* superblockBytes = (struct super_block*)read_data(boot_record_length, BLOCK_LENGTH);
    
//     if(strncmp(superblockBytes->format_magic, "ext4", 4) != 0)
//     {
//         ASSERT(FALSE, "DISK NOT FORMATTED.");
//         return;
//     }
    
//     disk_super_block = superblockBytes;
// }

// void save_super_block()
// {
//     uint8_t* superblockBytes = (uint8_t*)disk_super_block;
    
//     write_data(superblockBytes, BLOCK_LENGTH, SUPERBLOCK_START);
// }

// void load_inodes_map()
// {
//     uint8_t* bytes = read_data(INODE_MAP_START, sizeof(struct inode_map_entry) * MAX_FILES_NUM);
    
//     fs_inodes_map = (struct inode_map_entry*)bytes;
// }

// void save_inodes_map()
// {
//     uint8_t* bytes = (uint8_t*)fs_inodes_map;
    
//     write_data(bytes, INODE_MAP_START, sizeof(struct inode_map_entry) * MAX_FILES_NUM);
// }

// inode_num create_inode(char* name)
// {
//     struct inode* node = malloc(sizeof(struct inode));
//     node->inode_number = ++disk_super_block->inodesEntriesAmount;
//     node->file_type = 1;
//     node->size = 0;
//     node->flags = 0;
    
    
// }

// void map_inode(struct inode* node, char* name)
// {
//     *(fs_inodes_map + node->inode_number).fname = name;
//     *(fs_inodes_map + node->inode_number).inode_number = node->inode_number;    
// }

// void get_inode_available_address(inode* node)
// {
//     *(fs_inodes_map + node->inode_number).inode_disk_address = 0;
// }

// void write_inode(inode_num inode_number, uint8_t* data, uint32_t length)
// {
    
// }

// uint8_t* read_inode(inode_num inode_number, uint32_t* length)
// {
    
// }
