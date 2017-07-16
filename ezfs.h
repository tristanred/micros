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
file_h ezfs_find_file(file_h file, char* name);
size_t ezfs_read_file(file_h file, uint8_t* buf);
size_t ezfs_write_file(file_h file, uint8_t* buf, size_t bufLen);
void ezfs_rename_file(file_h file, char* toName);
void ezfs_protect_file(file_h file, enum FS_FILE_ACCESS access);
void ezfs_delete_file(file_h file);

// EZ-FS Functions
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

// Allocations management functions
void ezfs_write_allocation_to_disk(struct file_allocation* file);
struct file_allocation* ezfs_find_file_info(file_h file);





// void load_super_block();

// void save_super_block();

// void load_inodes_map();

// void save_inodes_map();

// inode_num create_inode(char* name[4];;

// void map_inode(struct inode* node);

// void get_inode_available_address(struct inode* node);

// void write_inode(inode_num inode_number, uint8_t* data, uint32_t length);

// uint8_t* read_inode(inode_num inode_number, uint32_t* length);

#endif
