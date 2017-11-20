#ifndef PCI_H
#define PCI_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"

#include "pci_class.h"

#define PCI_INDEX_PORT 0xCF8
#define PCI_DATA_PORT 0xCFC

// PCI Overview
// 256 available buses
// 32 devices on each bus
// 8 functions per device
// 256 bytes of data on each function

struct pci_device 
{
    BOOL valid_device;
    
    // Location information
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    
    // Register 0x00
    uint16_t deviceID;
    uint16_t vendorID;
    // Register 0x04
    uint16_t status;
    uint16_t command;
    // Register 0x08
    uint8_t classCode;
    uint8_t subClass;
    uint8_t progIF;
    uint8_t revID;
    // Register 0x0C
    uint8_t BIST;
    uint8_t headerType;
    uint8_t latencyTimer;
    uint8_t cacheLineSize;
    // Register 0x10
    uint32_t barAddress0;
    // Register 0x14
    uint32_t barAddress1;
    // Register 0x18
    uint32_t barAddress2;
    // Register 0x1C
    uint32_t barAddress3;
    // Register 0x20
    uint32_t barAddress4;
    // Register 0x24
    uint32_t barAddress5;
    // Register 0x28
    uint32_t cardbusCISPointer;
    // Register 0x2C
    uint16_t subsystemID;
    uint16_t subsystemVendorID;
    // Register 0x30
    uint32_t expansionROMBaseAddress;
    // Register 0x34
    uint16_t reservedOne;
    uint8_t reservedTwo;
    uint8_t capabilitiesPointer;
    // Register 0x38
    uint32_t reservedThree;
    // Register 0x3C
    uint8_t maxLatency;
    uint8_t minGrant;
    uint8_t intPin;
    uint8_t intLine;
};

struct pci_request
{
    struct pci_device* dev;
    
    uint8_t reg_num;
};

struct pci_device get_device(uint8_t bus, uint8_t device, uint8_t function);

struct pci_device** get_devices_list();

uint32_t build_request(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg);

void print_pci_device_info(struct pci_device* device);

#endif
