#ifndef EZ_FS_H
#define EZ_FS_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "fs_types.h"

// File amounts
#define MAX_FILES_NUM 10000
#define ROOT_LIST_AMOUNT 12

#define MAX_FILE_NAME 256

#define BLOCK_SIZE 1024
#define METABLOCK_ADDRESS 512


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
    char name[MAX_FILE_NAME];
    uint64_t dataBlockDiskAddress;
    uint32_t fileNumber;
    uint32_t size;
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

void ez_format_disk();

struct filesystem_metablock* ezfs_create_metablock();
struct filesystem_metablock* ezfs_load_disk_metablock();
void ezfs_write_metablock(struct filesystem_metablock* block);

void ezfs_format_allocation_area();
void ezfs_load_disk_allocation_area();
void ezfs_write_allocation_area();

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
