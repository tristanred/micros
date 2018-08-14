#ifndef ATA_MONITOR_H
#define ATA_MONITOR_H

#include "common.h"
#include "ata.h"

#include <stddef.h>
#include <stdint.h>

struct kernel_info_block;

#define ATA_DEFAULT_TIMEOUT 5

enum ata_driver_status
{
    ATA_OK,
    ATA_ERROR
};

enum ata_error
{
    ATA_ERROR_OK = 0,
    ATA_ERROR_IO_TIMEOUT,
    ATA_ERROR_NOSELECT,            // When no disk are targeted for an operation
    ATA_ERROR_BADPORT,
    ATA_ERROR_FAILED
};

enum ata_disk_select
{
    ATA_MASTER_0,
    ATA_SLAVE_0,
    ATA_MASTER_1,
    ATA_SLAVE_1,
    ATA_NONE
};

struct ata_driver_info
{
    uint16_t currentDiskPort;
    enum ata_disk_select currentDisk;
    
    struct ata_identify_device* device_info;
    
    BOOL driverError;
};
struct ata_driver_info* ata_driver;


void init_module_ata_driver(struct kernel_info_block* kinfo);

void test_io_port();

unsigned char get_status();

uint8_t* read_data(uint64_t startAddress, uint64_t length);
void write_data(uint8_t* data, uint64_t length, uint64_t startAddress);
uint64_t get_sector_from_address(uint64_t address, uint16_t* sectorOffset);



enum ata_driver_status driver_ata_get_status();

int driver_ata_wait_for_clear_bit(unsigned char statusBits, uint32_t timeout);
int driver_ata_wait_for_set_bit(unsigned char statusBits, uint32_t timeout);
int driver_ata_wait_for_only_set_bit(unsigned char statusBits, uint32_t timeout);

int driver_ata_select_drive(enum ata_disk_select disk);

int driver_ata_select_drive_with_lba_bits(enum ata_disk_select disk, uint8_t top_lba_bits);

void driver_ata_resetdisk();

int driver_ata_identify(struct ata_identify_device* drive_info);

void driver_ata_flush_cache();

uint16_t* driver_ata_read_sectors(uint8_t sectorCount, uint64_t startingSector);

void driver_ata_verify_sectors(uint8_t sectorCount, uint64_t startingSector);

void driver_ata_write_sectors(uint16_t* data, uint8_t sectorCount, uint64_t startingSector);

void driver_ata_write_test_sectors();

// ATA Functions upgrade
BOOL driver_ata_valid_disk();

void driver_ata_log(const char* msg);

#endif
