#ifndef PCI_NAMES
#define PCI_NAMES

/* This file lists the PCI classes/subclasses/program-interfaces */

/* PCI CLASSES */
const char classnames[22][64] = {
    "Unclassed",
    "Mass Storage",
    "Net Controller",
    "Display Controller",
    "Multimedia Controller",
    "Memory Controller",
    "Bridge",
    "Communication Controller",
    "Generic System",
    "Input Device Controller",
    "Dock Station",
    "Processor",
    "Serial Bus Controller",
    "Wireless Controller",
    "Intelligent Controller",
    "Satellite Communication Controller",
    "Encryption Controller",
    "Signal Processor Controller",
    "Processor Accelerator",
    "Non Essential Instrumentation",
    "CoProcessor",
    "Unassigned"
};

/* PCI MASS STORAGE 0x01 SUBCLASSES */
const char massstoragenames[10][64] = {
    "SCSI Controller",
    "IDE Interface",
    "Floppy Controller",
    "IPI Bus Controller",
    "RAID Bus Controller",
    "ATA Controller",
    "SATA Controller",
    "Serial SCSI Controller",
    "NVOL Memory Controller",
    "Generic Storage Controller"
};

/* PCI NETWORK CONTROLLER 0x02 SUBCLASSES */
const char netcontrollernames[10][64] = {
    "Ethernet controller",
    "Token Ring Controller",
    "FDDI Controller",
    "ATSM Controller",
    "IDSN Controller",
    "WORLDFIP Controller",
    "PICMG Controller",
    "Infiniband Controller",
    "Fabric Controller",
    "Generic Net Controller"
};

/* PCI DISPLAY 0x03 SUBCLASSES */
const char displaynames[4][64] = {
    "VGA Controller",
    "XGA Controller",
    "3D Controller",
    "Generic Display Controller"
};

/* PCI MULTIMEDIA 0x04 SUBCLASSES */
const char multimediacontrollernames[5][64] = {
    "Video Controller",
    "Audio Controller",
    "Telephony Device",
    "Media Audio Device",
    "Generic Media Controller"
};

/* PCI MEMORY 0x05 SUBCLASSES */
const char memorynames[3][64] = {
    "RAM",
    "Flash RAM",
    "Generic Memory Controller"
};

/* PCI BRIDGE 0x06 SUBCLASSES */
const char bridgenames[12][64] = {
    "Host",
    "ISA",
    "EISA",
    "MicroChannel",
    "PCI",
    "PCMCIA",
    "NUBUS",
    "CardBus",
    "Raceway",
    "PCI to PCI",
    "Infiniband",
    "Generic Bridge"
};

/* PCI COMMUNICATION 0x07 SUBCLASSES */
const char communicationnames[7][64] = {
    "Serial Controller",
    "Paralel Controller",
    "Multiport Serial Controller",
    "Modem Controller",
    "GPIB Controller",
    "Smartcard Controller",
    "Serial Generic Controller"
};

/* PCI GENERIC SYS 0x08 SUBCLASSES */
const char systemnames[8][64] = {
    "PIC",
    "DMA Controller",
    "Timer",
    "RTC",
    "PCI Hotplug Adapter",
    "SD Host Controller",
    "IO MMU",
    "Generic System Peripheral"
};

/* PCI INPUT DEVICE 0x09 */
const char inputnames[5][64] = {
    "Keyboard Controller",
    "Digitizer Pen",
    "Mouse Controller",
    "Scanner Controller",
    "Generic Input Controller"
};

/* PCI SERIAL BUS CONTROLLER 0xc */
const char serialbusnames[10][64] = {
    "Firewire",
    "AccessBus",
    "SSA",
    "USB",
    "Fibre Channel",
    "SMBUS",
    "Infiniband",
    "IPMI SMIC",
    "SERCOS",
    "CANBUS"
};

#endif
