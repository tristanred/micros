#ifndef AHCI_DRIVER_H
#define AHCI_DRIVER_H

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
#define AHCI_CAP_S64A   (x) (x & 1<<31) // #31 Supports 64-bit Addressing
#define AHCI_CAP_SNCQ   (x) (x & 1<<30) // #30 Supports Native Command Queuing
#define AHCI_CAP_SSNTF  (x) (x & 1<<29) // #29 Supports SNotification Register
#define AHCI_CAP_SMPS   (x) (x & 1<<28) // #28 Supports Mechanical Presence Switch
#define AHCI_CAP_SSS    (x) (x & 1<<27) // #27 Supports Staggered Spin-up
#define AHCI_CAP_SALP   (x) (x & 1<<26) // #26 Supports Aggressive Link Power Management
#define AHCI_CAP_SAL    (x) (x & 1<<25) // #25 Supports Activity LED
#define AHCI_CAP_SCLO   (x) (x & 1<<24) // #24 Supports Command List Override
#define AHCI_CAP_ISS    (x) (x & 0xF00000>>20) // #23:20 Interface Speed Support
#define AHCI_CAP_SAM    (x) (x & 1<<18) // #18 Supports AHCI mode only
#define AHCI_CAP_SPM    (x) (x & 1<<17) // #17 Supports Port Multiplier
#define AHCI_CAP_FBSS   (x) (x & 1<<16) // #16 FIS-based Switching Supported
#define AHCI_CAP_PMD    (x) (x & 1<<15) // #15 PIO Multiple DRQ Block
#define AHCI_CAP_SSC    (x) (x & 1<<14) // #14 Slumber State Capable
#define AHCI_CAP_PSC    (x) (x & 1<<13) // #13 Partial State Capable
#define AHCI_CAP_NCS    (x) (x & 1<<12) // #12:08 Number of Command Slots
#define AHCI_CAP_CCCS   (x) (x & 1<<7)  // #07 Command Completion Coalescing Supported
#define AHCI_CAP_EMS    (x) (x & 1<<6)  // #06 Enclosure Management Supported
#define AHCI_CAP_SXS    (x) (x & 1<<5)  // #05 Supports External SATA
#define AHCI_CAP_SXS    (x) (x & 0x1F)  // #04:00 Number of Ports

// AHCI GHC Bytes
#define AHCI_GHC_AE     (x) (x & 1<<31) // #31 AHCI Enable
#define AHCI_GHC_MRSM   (x) (x & 1<<2)  // #02 MSI Revert to Single Message
#define AHCI_GHC_IE     (x) (x & 1<<1)  // #01 Interrupt Enable
#define AHCI_GHC_HR     (x) (x & 1)  // #00 HBA Reset

#endif
