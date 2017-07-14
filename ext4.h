#ifndef EXT4_H
#define EXT4_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"

// File amounts
#define MAX_FILES_NUM 10000
#define ROOT_LIST_AMOUNT 12

// Disk addresses
#define BOOT_RECORD_START 0
#define BOOT_RECORD_LENGTH 512
#define SUPERBLOCK_START BOOT_RECORD_LENGTH
#define BLOCK_LENGTH 1024
#define INODE_MAP_START SUPERBLOCK_START + BLOCK_LENGTH

typedef uint64_t inode_num;

struct inode_map_entry
{
    char fname[256];
    inode_num inode_number;
    uint64_t inode_disk_address;
};
struct inode_map_entry* fs_inodes_map;

struct super_block
{
    char format_magic[5];
    
    uint64_t inodesEntriesAmount;
    
    uint64_t inodes_map_disk_addr;
    
    uint64_t inodes_data_area_start;
    
    uint8_t padding[996];
};
struct super_block* disk_super_block;


struct root_blocks_list
{
    uint64_t block_addresses[ROOT_LIST_AMOUNT];
    uint64_t one_indirection_table;
    uint64_t two_indirection_table;
    uint64_t three_indirection_table;
};

struct inode
{
    inode_num inode_number;
    uint8_t file_type;
    uint32_t size;
    uint32_t created_date;
    uint32_t flags;
    
    struct root_blocks_list root_blocks;
};
struct inode* inodes_data;

#define boot_record_length 512
#define super_block_length 1024

void format_disk();

void load_super_block();

void save_super_block();

void load_inodes_map();

void save_inodes_map();

inode_num create_inode(char* name);

void write_inode(inode_num inode_number, uint8_t* data, uint32_t length);

uint8_t* read_inode(inode_num inode_number, uint32_t* length);

#endif
