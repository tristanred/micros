#ifndef EZFS_TYPES_H
#define EZFS_TYPES_H

// File amounts
#define MAX_FILES_NUM 10
#define ROOT_LIST_AMOUNT 12

#define MAX_FILE_NAME 256

#define BLOCK_SIZE 1024
#define METABLOCK_ADDRESS 512

#define DEFAULT_FILE_SIZE 1024

#define SIZEOF_filesystem_metablock
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
} __attribute__((packed));

struct file_allocation
{
    int allocated;
    char name[MAX_FILE_NAME];
    uint64_t dataBlockDiskAddress;
    uint32_t fileNumber;
    int id;
    uint32_t dataSize;
    uint32_t diskSize;
    int32_t type;
} __attribute__((packed));


#endif // EZFS_TYPES_H
