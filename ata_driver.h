#ifndef ATA_MONITOR_H
#define ATA_MONITOR_H

#include "common.h"

#include <stddef.h>
#include <stdint.h>

struct kernel_info_block;

#define ATA_DEFAULT_TIMEOUT 5

// START ATA Spec values
#define DRIVE_PORT 0x1F0

// Registers offset
#define DATA_PORT 0x0
#define FEAT_ERRO 0x1
#define SEC_COUNT 0x2
#define SEC_NUM 0x3
#define LBA_LOW 0x3
#define CYL_LOW 0x4
#define LBA_MID 0x4
#define CYL_HIGH 0x5
#define LBA_HIGH 0x5
#define DRIVE_HEAD 0x6
#define COMMAND_REG_STATUS 0x7

// Status flags
#define STATUS_BUSY 0x80
#define STATUS_READY 0x40
#define STATUS_FAULT 0x20           // STATUS_FAULT and STATUS_STREAM ERROR
#define STATUS_STREAM_ERROR 0x20    // share the same bit.
#define STATUS_DEF_WRITE_ERROR 0x10
#define STATUS_DATA_REQUEST 0x8
#define STATUS_ALIGN_ERROR 0x4
#define STATUS_SENSE_DATA 0x2
#define STATUS_ERROR 0x1

#define PRIMARY_PORT 0x1F0
#define SECONDARY_PORT 0x170

#define MASTER_PORT_SELECTOR 0xE0
#define SLAVE_PORT_SELECTOR 0xF0

#define PRIMARY_ALTERNATE_PORT 0x3F6
#define SECEONDARY_ALTERNATE_PORT 0x376
// END ATA Spec values

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
    ATA_ERROR_FAILED
};

enum ata_disk_select
{
    ATA_NONE,
    ATA_MASTER_0,
    ATA_SLAVE_0,
    ATA_MASTER_1,
    ATA_SLAVE_1
};

#pragma pack(1)
struct ata_identify_device
{
    uint16_t generalConfig;                                                //0
    uint16_t obsolete_1;                                                   //1
    uint16_t specificConfig;                                               //2
    uint16_t obsolete_3;                                                   //3
    uint16_t retired_4[2];                                                 //4
    uint16_t obsolete_6;                                                   //6
    uint16_t reservedCFA_7[2];                                             //7
    uint16_t retired_9;                                                    //9
    uint16_t serialNumber[10];                                             //10
    uint16_t retired_20[2];                                                //20
    uint16_t obsolete_22;                                                  //22
    uint16_t firmwareRev[4];                                               //23
    uint16_t modelNumber[20];                                              //27
    uint16_t obsolete_47;                                                  //47
    uint16_t trusted_computing_options;                                    //48
    uint16_t capabilities_49;                                              //49
    uint16_t capabilities_50;                                              //50
    uint16_t obsolete_51[2];                                               //51
    uint16_t freefall_high;                                                //53
    uint16_t obsolete_54[5];                                               //54
    uint16_t advanced_feature_support;                                     //59
    uint32_t addressable_sectors_lba28;                                    //60
    uint16_t obsolete_62;                                                  //62
    uint16_t multiword_dma_info;                                           //63
    uint16_t pio_mode_support;                                             //64
    uint16_t min_multiword_dma_tpw;                                        //65
    uint16_t reccomanded_multiword_dma_tpw;                                //66
    uint16_t min_pio_transfer_cycle_without_flowcontrol;                   //67
    uint16_t min_pio_transfer_cycle_with_flowcontrol;                      //68
    uint16_t additional_supported;                                         //69
    uint16_t reserved_70;                                                  //70
    uint16_t reserved_identify_packet_device[4];                           //71
    uint16_t queue_depth;                                                  //75
    uint16_t serial_ata_capabilities;                                      //76
    uint16_t serial_ata_additional_capabilities;                           //77
    uint16_t serial_ata_features_supported;                                //78
    uint16_t serial_ata_features_enabled;                                  //79
    uint16_t major_version_number; // Contains the ACS support bits        //80
    uint16_t minor_version_number;                                         //81
    uint16_t commands_and_featuressets_supported_82;                       //82
    uint16_t commands_and_featuressets_supported_83;                       //83
    uint16_t commands_and_featuressets_supported_84;                       //84
    uint16_t commands_and_featuressets_supported_or_enabled_85;            //85
    uint16_t commands_and_featuressets_supported_or_enabled_86;            //86
    uint16_t commands_and_featuressets_supported_or_enabled_87;            //87
    uint16_t ultra_dma_modes;                                              //88
    uint16_t secure_erase_extended_time_89;                                //89
    uint16_t secure_erase_extended_time_90;                                //90
    uint16_t current_apm_level_values;                                     //91
    uint16_t master_password_identifier;                                   //92
    uint16_t hardware_reset_results;                                       //93
    uint16_t obsolete_94;                                                  //94
    uint16_t stream_min_req_size;                                          //95
    uint16_t stream_dma_transfer_time;                                     //96
    uint16_t stream_access_latency;                                        //97
    uint32_t stream_perf_granularity;                                      //98
    uint64_t user_addressable_sectors_count;                               //100
    uint16_t pio_streaming_transfer_time;                                  //104
    uint16_t max_number_512b_blocks_dataset_management;                    //105
    uint16_t physical_logical_sector_size;                                 //106
    uint16_t interseek_delay_accoustic_testing;                            //107
    uint16_t worldwide_name[4];                                            //108
    uint16_t reserved_112[4];                                              //112
    uint16_t obsolete_116;                                                 //116
    uint32_t logical_sector_size;                                          //117
    uint16_t commands_and_featuressets_supported_119;                      //118
    uint16_t commands_and_featuressets_supported_or_enabled_120;           //119
    uint16_t reserved_121[6];                                              //120
    uint16_t obsolete_127;                                                 //127
    uint16_t security_status;                                              //128
    uint16_t vendor_specific_129[31];                                      //129
    uint16_t reserved_CFA_160[8];                                          //160
    uint16_t device_nominal_form_factor;                                   //168
    uint16_t dataset_management_support;                                   //169
    uint16_t additional_product_identifier[4];                             //170
    uint16_t reserved_174[2];                                              //174
    uint16_t current_media_number[30];                                     //176
    uint16_t sct_command_transport;                                        //206
    uint16_t reserved_207[2];                                              //207
    uint16_t aligment_of_log_in_phys_sector;                               //209
    uint32_t write_read_verify_mode_3_count;                               //210
    uint32_t write_read_verify_mode_2_count;                               //212
    uint16_t obsolete_214[3];                                              //214
    uint16_t nominal_rotation_rate;                                        //217
    uint16_t reserved_218;                                                 //218
    uint16_t obsolete_219;                                                 //219
    uint16_t write_read_verify_current_featureset_mode;                    //220
    uint16_t reserved_221;                                                 //221
    uint16_t transport_major_version;                                      //222
    uint16_t transport_minor_version;                                      //223
    uint16_t reserved_224[6];                                              //224
    uint64_t extended_user_addressable_sectors_count;                      //230
    uint16_t microcode_min_datablocks;                                     //234
    uint16_t microcode_max_datablocks;                                     //235
    uint16_t reserved_236[19];                                             //236
    uint16_t integrity;                                                    //255
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

void driver_ata_select_drive(enum ata_disk_select disk);

void driver_ata_select_drive_with_lba_bits(enum ata_disk_select disk, uint8_t top_lba_bits);

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
