#include "ext4.h"

#include "ata_driver.h"
#include "memory.h"
#include "array_utils.h"
#include "string.h"

void format_disk()
{
    uint64_t start_superblock_address = boot_record_length;
    uint8_t blankData[super_block_length];
    array_set(blankData, 0, super_block_length);
    
    write_data(blankData, super_block_length, start_superblock_address);
    
    struct super_block* superblock = malloc(sizeof(struct super_block));
    sprintf_1d(superblock->format_magic, "ext44\0", 0);
    superblock->inodesEntriesAmount = 0;
    superblock->inodes_map_disk_addr = start_superblock_address + super_block_length;
    superblock->inodes_data_area_start = superblock->inodes_map_disk_addr + (sizeof(struct inode_map_entry) * MAX_FILES_NUM);
    
    memcpy(blankData, (uint8_t*)superblock, sizeof(struct super_block));
    
    write_data(blankData, super_block_length, start_superblock_address);
    
    disk_super_block = superblock;
}

void load_super_block()
{
    struct super_block* superblockBytes = (struct super_block*)read_data(boot_record_length, BLOCK_LENGTH);
    
    if(strncmp(superblockBytes->format_magic, "ext4", 4) != 0)
    {
        ASSERT(FALSE, "DISK NOT FORMATTED.");
        return;
    }
    
    disk_super_block = superblockBytes;
}

void save_super_block()
{
    uint8_t* superblockBytes = (uint8_t*)disk_super_block;
    
    write_data(superblockBytes, BLOCK_LENGTH, SUPERBLOCK_START);
}

void load_inodes_map()
{
    uint8_t* bytes = read_data(INODE_MAP_START, sizeof(struct inode_map_entry) * MAX_FILES_NUM);
    
    fs_inodes_map = (struct inode_map_entry*)bytes;
}

void save_inodes_map()
{
    uint8_t* bytes = (uint8_t*)fs_inodes_map;
    
    write_data(bytes, INODE_MAP_START, sizeof(struct inode_map_entry) * MAX_FILES_NUM);
}
