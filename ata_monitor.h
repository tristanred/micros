#ifndef ATA_MONITOR_H
#define ATA_MONITOR_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"

#define DRIVE_PORT 0x1F0

// Registers offset
#define DATA_PORT DRIVE_PORT + 0x0
#define FEAT_ERRO DRIVE_PORT + 0x1
#define SEC_COUNT DRIVE_PORT + 0x2
#define SEC_NUM DRIVE_PORT + 0x3
#define CYL_LOW DRIVE_PORT + 0x4
#define CYL_HIGH DRIVE_PORT + 0x5
#define DRIVE_HEAD DRIVE_PORT + 0x6
#define COMMAND_REG_STATUS DRIVE_PORT + 0x7

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

struct ata_identify_device
{
    uint16_t generalConfig;
    uint16_t obsolete_1;
    uint16_t specificConfig;
    uint16_t obsolete_3;
    uint16_t retired_4;
    uint16_t retired_5;
    uint16_t obsolete_6;
    uint16_t reservedCFA_7;
    uint16_t reservedCFA_8;
    uint16_t retired_9;
    uint16_t serialNumber[10];
    uint16_t retired_20;
    uint16_t retired_21;
    uint16_t obsolete_22;
    uint16_t firmwareRev[4];
    uint16_t modelNumber[20];
    uint16_t obsolete_47;
    uint16_t trusted_computing_options;
    uint16_t capabilities_49;
    uint16_t capabilities_50;
    uint16_t obsolete_51;
    uint16_t obsolete_52;
    uint16_t freefall_high;
    uint16_t obsolete_54[5];
    uint16_t advanced_feature_support;
    uint32_t addressable_sectors_lba28;
    uint16_t obsolete_62;
    uint16_t multiword_dma_info;
    uint16_t pio_mode_support;
    uint16_t min_multiword_dma_tpw;
    uint16_t reccomanded_multiword_dma_tpw;
    uint16_t min_pio_transfer_cycle_without_flowcontrol;
    uint16_t min_pio_transfer_cycle_with_flowcontrol;
    uint16_t additional_supported;
    uint16_t reserved_70;
    uint16_t reserved_identify_packet_device;
    uint16_t queue_depth;
    uint16_t serial_ata_capabilities;
    uint16_t serial_ata_additional_capabilities;
    uint16_t serial_ata_features_supported;
    uint16_t serial_ata_features_enabled;
    uint16_t major_version_number; // Contains the ACS support bits
    uint16_t minor_version_number;
    uint16_t commands_and_featuressets_supported_82;
    uint16_t commands_and_featuressets_supported_83;
    uint16_t commands_and_featuressets_supported_84;
    uint16_t commands_and_featuressets_supported_or_enabled_85;
    uint16_t commands_and_featuressets_supported_or_enabled_86;
    uint16_t commands_and_featuressets_supported_or_enabled_87;
    uint16_t ultra_dma_modes;
    uint16_t secure_erase_extended_time_89;
    uint16_t secure_erase_extended_time_90;
    uint16_t current_apm_level_values;
    uint16_t master_password_identifier;
    uint16_t hardware_reset_results;
    uint16_t obsolete_94;
    uint16_t stream_min_req_size;
    uint16_t stream_dma_transfer_time;
    uint16_t stream_access_latency;
    uint32_t stream_perf_granularity;
    uint64_t user_addressable_sectors_count;
    uint16_t pio_streaming_transfer_time;
    uint16_t max_number_512b_blocks_dataset_management;
    uint16_t physical_logical_sector_size;
    uint16_t interseek_delay_accoustic_testing;
    uint16_t worldwide_name[4];
    uint16_t reserved_112[4];
    uint16_t obsolete_116;
    uint32_t logical_sector_size;
    uint16_t commands_and_featuressets_supported_119;
    uint16_t commands_and_featuressets_supported_or_enabled_120;
    uint16_t reserved_121[6];
    uint16_t obsolete_127;
    uint16_t security_status;
    uint16_t vendor_specific_129[31];
    uint16_t reserved_CFA_160[8];
    uint16_t device_nominal_form_factor;
    uint16_t dataset_management_support;
    uint16_t additional_product_identifier[4];
    uint16_t reserved_174;
    uint16_t reserved_175;
    uint16_t current_media_number[30];
    uint16_t sct_command_transport;
    uint16_t reserved_207;
    uint16_t reserved_208;
    uint16_t aligment_of_log_in_phys_sector;
    uint32_t write_read_verify_mode_3_count;
    uint32_t write_read_verify_mode_2_count;
    uint16_t obsolete_214;
    uint16_t obsolete_215;
    uint16_t obsolete_216;
    uint16_t nominal_rotation_rate;
    uint16_t reserved_218;
    uint16_t obsolete_219;
    uint16_t write_read_verify_current_featureset_mode;
    uint16_t reserved_221;
    uint16_t transport_major_version;
    uint16_t transport_minor_version;
    uint16_t reserved_224[6];
    uint64_t extended_user_addressable_sectors_count;
    uint16_t microcode_min_datablocks;
    uint16_t microcode_max_datablocks;
    uint16_t reserved_236[19];
    uint16_t integrity;
};

void test_io_port();

void read_bytes(int count, unsigned char* buffer);

void io_wait();

void identify();

void wait_for_ready();

unsigned char get_status();


// ATA Driver section, will be migrated to its own file

void driver_ata_wait_for_clear_bit(unsigned char statusBits);

void driver_ata_wait_for_set_bit(unsigned char statusBits);

void driver_ata_wait_for_only_set_bit(unsigned char statusBits);

void driver_ata_update();

void driver_ata_resetdisk();

void driver_ata_identify(struct ata_identify_device* drive_info);

void driver_ata_flush_cache();

uint8_t* driver_ata_read_sectors(uint8_t sectorCount, uint32_t startingSector);

void driver_ata_verify_sectors(uint8_t sectorCount, uint32_t startingSector);

void driver_ata_write_sectors(uint16_t* data, uint8_t sectorCount, uint32_t startingSector);

#endif
