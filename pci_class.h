#ifndef PCI_CLASS
#define PCI_CLASS

/* This file lists the PCI classes/subclasses/program-interfaces */

/* PCI CLASSES */
#define PCI_CLA_UNCLASSED                           0x00
#define PCI_CLA_MASS_STORAGE                        0x01
#define PCI_CLA_NET_CONTROLLER                      0x02
#define PCI_CLA_DISPLAY_CONTROLLER                  0x03
#define PCI_CLA_MULTIMEDIA_CONTROLLER               0x04
#define PCI_CLA_MEMORY_CONTROLLER                   0x05
#define PCI_CLA_BRIDGE                              0x06
#define PCI_CLA_COMMUNICATION_CONTROLLER            0x07
#define PCI_CLA_GENERIC_SYS                         0x08
#define PCI_CLA_INPUT_DEV_CONTROLLER                0x09
#define PCI_CLA_DOCK_STATION                        0x0a
#define PCI_CLA_PROCESSOR                           0x0b
#define PCI_CLA_SERIAL_BUS_CONTROLLER               0x0c
#define PCI_CLA_WIRELESS_CONTROLLER                 0x0d
#define PCI_CLA_INTELLIGENT_CONTROLLER              0x0e
#define PCI_CLA_SAT_COMMUNICATION_CONTROLLER        0x0f
#define PCI_CLA_ENCRYPTION_CONTROLLER               0x10
#define PCI_CLA_SIG_PROC_CONTROLLER                 0x11
#define PCI_CLA_PROC_ACCELERATOR                    0x12
#define PCI_CLA_NON_ESSENTIAL_INSTRUMENTATION       0x13
#define PCI_CLA_COPROCESSOR                         0x40
#define PCI_CLA_UNASSIGNED                          0xff

/* PCI MASS STORAGE 0x01 SUBCLASSES */
#define PCI_SUB_MASS_STOR_SCSI_CONTROLLER           0x00
#define PCI_SUB_MASS_STOR_IDE_INTERFACE             0x01
#define PCI_SUB_MASS_STOR_FLOPPY_CONTROLLER         0x02
#define PCI_SUB_MASS_STOR_IPI_BUS_CONTROLLER        0x03
#define PCI_SUB_MASS_STOR_RAID_BUS_CONTROLLER       0x04
#define PCI_SUB_MASS_STOR_ATA_CONTROLLER            0x05
#define PCI_SUB_MASS_STOR_SATA_CONTROLLER           0x06
#define PCI_SUB_MASS_STOR_SERIAL_SCSI_CONTROLLER    0x07
#define PCI_SUB_MASS_STOR_NVOL_MEMORY_CONTROLLER    0x08
#define PCI_SUB_MASS_STOR_CONTROLLER                0x80

/* PCI NETWORK CONTROLLER 0x02 SUBCLASSES */
#define PCI_SUB_NETWORK_ETHERNET_CONTROLLER         0x00
#define PCI_SUB_NETWORK_TOKENRING_CONTROLLER        0x01
#define PCI_SUB_NETWORK_FDDI_CONTROLLER             0x02
#define PCI_SUB_NETWORK_ATSM_CONTROLLER             0x03
#define PCI_SUB_NETWORK_ISDN_CONTROLLER             0x04
#define PCI_SUB_NETWORK_WORLDFIP_CONTROLLER         0x05
#define PCI_SUB_NETWORK_PICMG_CONTROLLER            0x06
#define PCI_SUB_NETWORK_INFINIBAND_CONTROLLER       0x07
#define PCI_SUB_NETWORK_FABRIC_CONTROLLER           0x08
#define PCI_SUB_NETWORK_CONTROLLER                  0x80

/* PCI DISPLAY 0x03 SUBCLASSES */
#define PCI_SUB_DISPLAY_VGA_CONTROLLER              0x00
#define PCI_SUB_DISPLAY_XGA_CONTROLLER              0x01
#define PCI_SUB_DISPLAY_3D_CONTROLLER               0x02
#define PCI_SUB_DISPLAY_CONTROLLER                  0x03

/* PCI MULTIMEDIA 0x04 SUBCLASSES */
#define PCI_SUB_MEDIA_VIDEO_CONTROLLER              0x00
#define PCI_SUB_MEDIA_AUDIO_CONTROLLER              0x01
#define PCI_SUB_MEDIA_TELEPHONY_DEVICE              0x02
#define PCI_SUB_MEDIA_AUDIO_DEVICE                  0x03
#define PCI_SUB_MEDIA_CONTROLLER                    0x80

/* PCI MEMORY 0x05 SUBCLASSES */
#define PCI_SUB_MEM_RAM                             0x00
#define PCI_SUB_MEM_FLASH_RAM                       0x01
#define PCI_SUB_MEM_CONTROLLER                      0x80

/* PCI BRIDGE 0x06 SUBCLASSES */
#define PCI_SUB_BRIDGE_HOST                         0x00
#define PCI_SUB_BRIDGE_ISA                          0x01
#define PCI_SUB_BRIDGE_EISA                         0x02
#define PCI_SUB_BRIDGE_MICROCHANNEL                 0x03
#define PCI_SUB_BRIDGE_PCI                          0x04
#define PCI_SUB_BRIDGE_PCMCIA                       0x05
#define PCI_SUB_BRIDGE_NUBUS                        0x06
#define PCI_SUB_BRIDGE_CARDBUS                      0x07
#define PCI_SUB_BRIDGE_RACEWAY                      0x08
#define PCI_SUB_BRIDGE_PCI_TO_PCI                   0x09
#define PCI_SUB_BRIDGE_INFINIBAND                   0x0a
#define PCI_SUB_BRIDGE                              0x80

/* PCI COMMUNICATION 0x07 SUBCLASSES */
#define PCI_SUB_SERIAL_CONTROLLER                   0x00
#define PCI_SUB_PARALEL_CONTROLLER                  0x01
#define PCI_SUB_MULTIPORT_SERIAL_CONTROLLER         0x02
#define PCI_SUB_MODEM_CONTROLLER                    0x03
#define PCI_SUB_GPIB_CONTROLLER                     0x04
#define PCI_SUB_SMARTCARD_CONTROLLER                0x05
#define PCI_SUB_SERIAL_CONTROLLER                   0x80

/* PCI GENERIC SYS 0x08 SUBCLASSES */
#define PCI_SUB_GENERIC_SYS_PIC                     0x00
#define PCI_SUB_GENERIC_SYS_DMA_CONTROLLER          0x01
#define PCI_SUB_GENERIC_SYS_TIMER                   0x02
#define PCI_SUB_GENERIC_SYS_RTC                     0x03
#define PCI_SUB_GENERIC_SYS_PCI_HOTPLUG_ADAPTER     0x04
#define PCI_SUB_GENERIC_SYS_SD_HOST_CONTROLLER      0x05
#define PCI_SUB_GENERIC_SYS_IOMMU                   0x06
#define PCI_SUB_GENERIC_SYS_SYSTEM_PERIPHERAL       0x80

/* PCI INPUT DEVICE 0x09 */
#define PCI_SUB_INPUT_KEYBOARD_CONTROLLER           0x00
#define PCI_SUB_INPUT_DIGITIZER_PEN                 0x01
#define PCI_SUB_INPUT_MOUSE_CONTROLLER              0x02
#define PCI_SUB_INPUT_SCANNER_CONTROLLER            0x03
#define PCI_SUB_INPUT_CONTROLLER                    0x80

/* PCI SERIAL BUS CONTROLLER 0xc */
#define PCI_SUB_SERIAL_BUS_FIREWIRE                  0x00
#define PCI_SUB_SERIAL_BUS_ACCESSBUS                 0x01
#define PCI_SUB_SERIAL_BUS_SSA                       0x02
#define PCI_SUB_SERIAL_BUS_USB                       0x03
#define PCI_SUB_SERIAL_BUS_FIBRE_CHANNEL             0x04
#define PCI_SUB_SERIAL_BUS_SMBUS                     0x05
#define PCI_SUB_SERIAL_BUS_INFINIBAND                0x06
#define PCI_SUB_SERIAL_BUS_IPMI_SMIC                 0x07
#define PCI_SUB_SERIAL_BUS_SERCOS                    0x08
#define PCI_SUB_SERIAL_BUS_CANBUS                    0x09


#endif
