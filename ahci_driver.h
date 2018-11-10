#ifndef AHCI_DRIVER_H
#define AHCI_DRIVER_H

#include <stddef.h>
#include <stdint.h>

#include "kernel.h"
#include "ata.h"

// Generic ATA
#define ATA_STATUS_BUSY 0x80
#define ATA_STATUS_READY 0x40
#define ATA_STATUS_FAULT 0x20           // STATUS_FAULT and STATUS_STREAM ERROR
#define ATA_STATUS_STREAM_ERROR 0x20    // share the same bit.
#define ATA_STATUS_DEF_WRITE_ERROR 0x10
#define ATA_STATUS_DATA_REQUEST 0x8
#define ATA_STATUS_ALIGN_ERROR 0x4
#define ATA_STATUS_SENSE_DATA 0x2
#define ATA_STATUS_ERROR 0x1

// Generic Host Control registers

#define AHCI_CAP        0x00 // Host Capabilities
#define AHCI_GHC        0x04 // Global Host Control
#define AHCI_IS         0x08 // Interrupt Status
#define AHCI_PI         0x0C // Ports Implemented
#define AHCI_VS         0x10 // Version
#define AHCI_CCC_CTL    0x14 // Command Completion Coalescing Control
#define AHCI_CCC_PORTS  0x18 // Command Completion Coalsecing Ports
#define AHCI_EM_LOC     0x1C // Enclosure Management Location
#define AHCI_EM_CTL     0x20 // Enclosure Management Control
#define AHCI_CAP2       0x24 // Host Capabilities Extended
#define AHCI_BOHC       0x28 // BIOS/OS Handoff Control and Status

// AHCI_CAP Bytes
#define AHCI_CAP_S64A(x)    (x & 1<<31) // #31 Supports 64-bit Addressing
#define AHCI_CAP_SNCQ(x)    (x & 1<<30) // #30 Supports Native Command Queuing
#define AHCI_CAP_SSNTF(x)   (x & 1<<29) // #29 Supports SNotification Register
#define AHCI_CAP_SMPS(x)    (x & 1<<28) // #28 Supports Mechanical Presence Switch
#define AHCI_CAP_SSS(x)     (x & 1<<27) // #27 Supports Staggered Spin-up
#define AHCI_CAP_SALP(x)    (x & 1<<26) // #26 Supports Aggressive Link Power Management
#define AHCI_CAP_SAL(x)     (x & 1<<25) // #25 Supports Activity LED
#define AHCI_CAP_SCLO(x)    (x & 1<<24) // #24 Supports Command List Override
#define AHCI_CAP_ISS(x)     ((x & 0xF00000)>>20) // #23:20 Interface Speed Support
#define AHCI_CAP_SAM(x)     (x & 1<<18) // #18 Supports AHCI mode only
#define AHCI_CAP_SPM(x)     (x & 1<<17) // #17 Supports Port Multiplier
#define AHCI_CAP_FBSS(x)    (x & 1<<16) // #16 FIS-based Switching Supported
#define AHCI_CAP_PMD(x)     (x & 1<<15) // #15 PIO Multiple DRQ Block
#define AHCI_CAP_SSC(x)     (x & 1<<14) // #14 Slumber State Capable
#define AHCI_CAP_PSC(x)     (x & 1<<13) // #13 Partial State Capable
#define AHCI_CAP_NCS(x)     (x & 0x1F00) // #12:08 Number of Command Slots
#define AHCI_CAP_CCCS(x)    (x & 1<<7)  // #07 Command Completion Coalescing Supported
#define AHCI_CAP_EMS(x)     (x & 1<<6)  // #06 Enclosure Management Supported
#define AHCI_CAP_SXS(x)     (x & 1<<5)  // #05 Supports External SATA
#define AHCI_CAP_NP(x)      (x & 0x1F)  // #04:00 Number of Ports

// AHCI GHC Bytes
#define AHCI_GHC_AE(x)      (x & 1<<31) // #31 AHCI Enable
#define AHCI_GHC_MRSM(x)    (x & 1<<2)  // #02 MSI Revert to Single Message
#define AHCI_GHC_IE(x)      (x & 1<<1)  // #01 Interrupt Enable
#define AHCI_GHC_HR(x)      (x & 1)  // #00 HBA Reset

// AHCI Port's Command and Status
#define AHCI_PxCMD_ICC(x)   ((x & 0xF0000000) >> 28) // #32:28 Interface Communication Control
#define AHCI_PxCMD_ASP(x)   (x & 1<<27) // #27 Aggressive Slumber / Partial
#define AHCI_PxCMD_ALPE(x)  (x & 1<<26) // #26 Aggressive Link Power Management Enable
#define AHCI_PxCMD_DLAE(x)  (x & 1<<25) // #25 Drive LED on ATAPI Enable
#define AHCI_PxCMD_ATAPI(x) (x & 1<<24) // #24 Device is ATAPI
#define AHCI_PxCMD_APSTE(x) (x & 1<<23) // #23 Automatic Partial to Slumber Transitions Enabled
#define AHCI_PxCMD_FBSCP(x) (x & 1<<22) // #22 FIS-based Switching Capable Port
#define AHCI_PxCMD_ESP(x)   (x & 1<<21) // #21 External SATA Port
#define AHCI_PxCMD_CPD(x)   (x & 1<<20) // #20 Cold Presence Detection
#define AHCI_PxCMD_MPSP(x)  (x & 1<<19) // #19 Mechanical Presence Switch Attached to Port
#define AHCI_PxCMD_HPCP(x)  (x & 1<<18) // #18 Hot Plug Capable Port
#define AHCI_PxCMD_PMA(x)   (x & 1<<17) // #17 Port Multiplier Attached
#define AHCI_PxCMD_CPS(x)   (x & 1<<16) // #16 Cold Presence State
#define AHCI_PxCMD_CR(x)    (x & 1<<15) // #15 Command List Running
#define AHCI_PxCMD_FR(x)    (x & 1<<14) // #14 FIS Receive Running
#define AHCI_PxCMD_MPSS(x)  (x & 1<<13) // #13 Mechanical Presence Switch State
#define AHCI_PxCMD_CSS(x)   ((x & 0x1F00) >> 8) // #12:08 Current Command Slot
#define AHCI_PxCMD_FRE(x)   (x & 1<<4) // #4 FIS Receive Enable
#define AHCI_PxCMD_CLO(x)   (x & 1<<3) // #3 Command List Override
#define AHCI_PxCMD_POD(x)   (x & 1<<2) // #2 Spin-Up Device
#define AHCI_PxCMD_SUD(x)   (x & 1<<1) // #2 Power On Device
#define AHCI_PxCMD_ST(x)    (x & 1)    // #1 Start

#pragma pack(1)
struct ahci_host_regs
{
    uint32_t host_capabilities;
    uint32_t global_host_control;
    uint32_t interrupt_status;
    uint32_t ports_implemented;
    uint32_t version;
    uint32_t command_completion_coalescing_control;
    uint32_t command_completion_coalescing_ports;
    uint32_t enclosure_management_location;
    uint32_t enclosure_management_control;
    uint32_t host_capabilities_extended;
    uint32_t bios_handoff_control_status;
};

#pragma pack(1)
struct ahci_vendor_regs
{
    uint32_t regs[6];
};

#define AHCI_SIG_ATA 0x00000101 // SATA Drive
#define AHCI_SIG_ATAPI 0xEB140101 // SATAPI device
#define AHCI_SIG_SEMB 0xC33C0101 // Enclosure management bridge
#define AHCI_SIG_PM 0x96690101 // Port Multiplier

#define AHCI_FIS_REG_H2D 0x27
#define AHCI_FIS_REG_D2H 0x34
#define AHCI_FIS_DMA_ACT 0x39
#define AHCI_FIS_DMA_SET 0x41
#define AHCI_FIS_DAT     0x46
#define AHCI_FIS_BIST    0x58
#define AHCI_FIS_PIO_SET 0x5F
#define AHCI_FIS_DEV_BIT 0xA1

#pragma pack(1)
struct ahci_port_regs
{
    uint32_t command_list_base_addr_lower;
    uint32_t command_list_base_addr_upper;
    uint32_t fis_base_addr_lower;
    uint32_t fis_base_addr_upper;
    uint32_t interrupt_status;
    uint32_t interrupt_enable;
    uint32_t command_and_status;
    uint32_t reserved1;
    uint32_t task_file_data;
    uint32_t signature;
    uint32_t serial_ata_status;
    uint32_t serial_ata_control;
    uint32_t serial_ata_error;
    uint32_t serial_ata_active;
    uint32_t serial_command_issue;
    uint32_t serial_ata_notification;
    uint32_t fis_based_switching_control;
    uint32_t device_sleep;
    uint32_t reserved2;
    uint32_t vendor_specific;
};

#pragma pack(1)
struct ahci_port_command_header
{
    uint16_t prdtl;
    uint16_t prflags;
    uint32_t prdByteCount;
    uint32_t commandtableBaseAddr;
    uint32_t commandtableBaseAddrUpper;
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t reserved3;
    uint32_t reserved4;
};

#pragma pack(1)
struct ahci_port_prdt
{
    uint32_t addr_base;
    uint32_t addr_upper;
    uint32_t reserved;
    uint32_t bytecount;
};

#pragma pack(1)
struct ahci_port_commandlist
{
    struct ahci_port_command_header entries[32];
};

#pragma pack(1)
struct ahci_port_commandtable
{
    uint8_t cmd_fis[64];
    uint8_t cmd_atapi[16];
    uint8_t reserved[48];

    struct ahci_port_prdt* regions; // 0 to 65535 elements
};

#pragma pack(1)
struct ahci_fis_reg_H2D
{
    uint8_t type; // 0x27
    uint8_t commandreg;
    uint8_t command;
    uint8_t features_low;

    uint8_t lba_1;
    uint8_t lba_2;
    uint8_t lba_3;
    uint8_t device;

    uint8_t lba_4;
    uint8_t lba_5;
    uint8_t lba_6;
    uint8_t features_high;

    uint8_t count_low;
    uint8_t count_high;
    uint8_t icc;
    uint8_t control;

    uint32_t reserved;
};

// Handy structure to keep the info about a disk.
struct ahci_disk_info
{
    uint8_t portNb;

    // BAR5 address
    uint32_t memory_addr;
};

/**
 * AHCI Memory overview
 *
 * The PCI bus device has the BAR5(ABAR) and it stores the address in memory
 * of the HBA.
 *
 * The HBA has generic registers and a set of register for each port.
 *
 * Each port has the address to a Command List and Received FIS struct.
 *
 * The Command list struct contains a list of command tables
 *
 */

#define AHCI_INVALID_PORT 255

struct pci_device;
struct pci_controlset;

struct ahci_driver_info
{
    struct pci_device* hba_device; // Host Bus Adapter

    uint32_t abar; // hba_device->barAddress5 shortcut

    uint8_t port_count;
    uint8_t disk_ports[32];
};
struct ahci_driver_info* ahci_driver;

/* Module Setup and init tasks */

/**
 * Kernel module init. Sets up the 'ahci_driver' object.
 */
void init_module_ahci_driver(struct kernel_info_block* kinfo);

/**
 * Scan the PCI bus and look for AHCI compatible disks. Right now this finds
 * SATA disks and adds the first one to the driver. Only the first disk found
 * will be used.
 */
int driver_ahci_find_disks(struct pci_controlset* pcs);

/**
 * Setup the AHCI structures in memory by setting the addresses in the
 * AHCI registers. This will allocate heap memory with some alignment
 * requirements.
 */
int driver_ahci_setup_memory(uint8_t portNb);

/**
 * For testing, returns whatever port we will always use for issuing
 * AHCI commands. This function looks for whatever port is connected to an
 * online disk.
 */
uint8_t driver_ahci_get_default_port();

/* Register reading */

/**
 * These functions return a copy of the targer registers. The structures can
 * be passed around safely without modifying the drive registers.
 */

int driver_ahci_read_GHC_regs(struct ahci_host_regs* regs);
int driver_ahci_read_port_regs(uint8_t portNb, struct ahci_port_regs* regs);
int driver_ahci_read_port_commandlist(uint8_t portNb, struct ahci_port_commandlist* data);
int driver_ahci_read_port_commandtable(uint8_t portNb, int commandNb, struct ahci_port_commandtable* data);

/* Register Access */

/**
 * These functions return the actual memory mapped registers. This is used to
 * modify the registers directly and read the registers values directly from the
 * controller.
 */

int driver_ahci_get_GHC_regs(struct ahci_host_regs** regs);
int driver_ahci_get_port_regs(uint8_t portNb, struct ahci_port_regs** regs);
int driver_ahci_get_port_commandlist(uint8_t portNb, struct ahci_port_commandlist** data);
int driver_ahci_get_port_commandtable(uint8_t portNb, int commandNb, struct ahci_port_commandtable** data);

/** Get the amount of ports supported by the machine not all of them may be
 * implemented. Not such a useful function because we can just call
 * driver_ahci_get_disk_ports and get the current amount of ports usable.
 */
int driver_ahci_get_ports_enabled();

/* Get the ports where a disk is present.
 * 'ports' must be initialized as an array of 32 ints all filled with 0.
 * 'amount' is the number of disks found, the first 'amount' elements of the 'ports'
 * array will be filled with the occupied port of that disk.
 */
int driver_ahci_get_disk_ports(uint8_t* ports, uint8_t* amount);

/**
 * Print each port's registers to the log.
 */
int driver_ahci_print_ports_info();

/* Drive Operation */
int driver_ahci_reset_controller();
int driver_ahci_reset_port(uint8_t portNb);
int driver_ahci_reset_all_ports();

/* IO Procedures */
int driver_ahci_read_data(uint8_t port, uint32_t addr_low, uint32_t addr_high, uint32_t length, uint8_t* buf);

int driver_ahci_identify(uint8_t port, struct ata_identify_device* data);

/* Command driving methods */
int driver_ahci_make_command_header(uint8_t portNb, uint8_t cmdslot, struct ahci_port_command_header** cmd);
int driver_ahci_make_command_fis(struct ahci_port_commandtable* cmdtable, struct ahci_fis_reg_H2D** fis);

int driver_ahci_get_next_cmdslot(uint8_t portNb, uint8_t* cmdslot);

#endif
