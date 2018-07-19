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

// TODO : Move to pcidev.h
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

/* 
 * This is the set of devices controlled by the kernel. It is meant to be 
 * allocated once and kept in a kernel module. The controlset can be rescanned
 * by calling pci_rescan_devices.
 */
struct pci_controlset
{
    int devicesCount;
    struct pci_device** deviceList;
};

/*
 * Kernel module for PCI bus
 */
struct pci_module
{
    BOOL module_loaded;
    BOOL module_active;
    struct pci_controlset* current_controlset;
    
};


// PCI Public interface

/*
 * This function enumerates the devices on the PCI bus and replace the current
 * controlset in the pci kernel module. If a new device 
 */
void pci_rescan_devices();

struct pci_controlset* get_devices_list(int* count);

void print_pci_device_info(struct pci_device* device);

// PCI Internal interface
struct pci_device get_device(uint8_t bus, uint8_t device, uint8_t function);

struct pci_device** pci_scan_bus(int* count);

uint32_t build_request(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg);

const char* pci_class_name(uint8_t pci_class);
const char* pci_subclass_name(uint8_t pci_class, uint8_t subclass);

#endif
