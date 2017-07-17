#ifndef EZ_FS_H
#define EZ_FS_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "fs_types.h"

// File amounts
#define MAX_FILES_NUM 10
#define ROOT_LIST_AMOUNT 12

#define MAX_FILE_NAME 256

#define BLOCK_SIZE 1024
#define METABLOCK_ADDRESS 512

#define DEFAULT_FILE_SIZE 1024

// Options
#define ZERO_ON_DELETE
#define ZERO_ON_RELOCATE

struct filesystem_metablock
{
    char magic[4];
    uint32_t files_amount;
    
    uint64_t allocation_area_start;
    uint64_t allocation_area_length;
    
    uint64_t data_area_start;
    uint64_t data_area_length; // Typically to end of disk
    
    uint8_t version_fs_major;
    uint8_t version_fs_minor;
    
    uint8_t padding[982];
};

struct file_allocation
{
    BOOL allocated;
    char name[MAX_FILE_NAME];
    uint64_t dataBlockDiskAddress;
    uint32_t fileNumber;
    file_h id;
    uint32_t dataSize;
    uint32_t diskSize;
    int32_t type;
};

struct filesystem_metablock* loaded_metablock;
struct file_allocation* allocated_files;

// EZ-FS Filesystem api
file_h ezfs_create_file(file_h dir, char* name, enum FS_FILE_ACCESS access, enum FS_FILE_FLAGS flags);
file_h ezfs_find_file(char* name);
size_t ezfs_read_file(file_h file, uint8_t** buf);
size_t ezfs_write_file(file_h file, uint8_t* buf, size_t bufLen);
void ezfs_rename_file(file_h file, char* toName);
void ezfs_protect_file(file_h file, enum FS_FILE_ACCESS access);
void ezfs_delete_file(file_h file);

// EZ-FS Internal Functions

// Disk formatting functions
void ezfs_prepare_disk();
void ezfs_format_disk();
void ezfs_format_allocation_area();
void ezfs_load_disk_allocation_area();
void ezfs_write_allocation_area();
struct filesystem_metablock* ezfs_create_metablock();
struct filesystem_metablock* ezfs_load_disk_metablock();
void ezfs_write_metablock(struct filesystem_metablock* block);

// Files data layout function
uint64_t ezfs_find_free_space(size_t size);
size_t ezfs_get_free_space_between_files(struct file_allocation* one, struct file_allocation* two);
BOOL ezfs_data_can_grow(struct file_allocation* file, size_t required);
BOOL ezfs_data_relocate(struct file_allocation* file, size_t required);
uint64_t ezfs_calculate_new_padded_size(uint64_t minSize);

// Allocations management functions
void ezfs_write_allocation_to_disk(struct file_allocation* file);
struct file_allocation* ezfs_find_file_info(file_h file);
void ezfs_deallocate(struct file_allocation* file);
void ezfs_sort_allocations();

// Data management functions
void ezfs_zero_file(struct file_allocation* file);
void ezfs_copy_data(struct file_allocation* file, uint64_t diskAddr);

#endif
