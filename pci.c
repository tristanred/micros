#include "pci.h"

#include "io_func.h"
#include "kernel_log.h"
#include "memory.h"
#include "vector.h"
#include "pci_class_names.h"

struct pci_device get_device(uint8_t bus, uint8_t device, uint8_t function)
{
    struct pci_device dev;
    dev.bus = bus;
    dev.device = device;
    dev.function = function;
    
    uint32_t request = build_request(bus, device, function, 0x00);
    outdw(PCI_INDEX_PORT, request);
    uint32_t result = indw(PCI_DATA_PORT);
    
    dev.deviceID = result >> 16;
    dev.vendorID = result & 0xFFFF;
    
    if(dev.vendorID == 0xFFFF)
    {
        dev.valid_device = FALSE;
        
        return dev;
    }
    else
    {
        dev.valid_device = TRUE;
    }
    
    request = build_request(bus, device, function, 0x04);
    outdw(PCI_INDEX_PORT, request);
    result = indw(PCI_DATA_PORT);
    
    dev.status = result >> 16;
    dev.command = result & 0xFFFF;
    
    request = build_request(bus, device, function, 0x08);
    outdw(PCI_INDEX_PORT, request);
    result = indw(PCI_DATA_PORT);
    
    dev.classCode = result >> 24;
    dev.subClass = (result & 0xFF0000) >> 16;
    dev.progIF = (result & 0xFF00) >> 8;
    dev.revID = (result & 0xFF);
    
    request = build_request(bus, device, function, 0x0C);
    outdw(PCI_INDEX_PORT, request);
    result = indw(PCI_DATA_PORT);
    
    dev.BIST = result >> 24;
    dev.headerType = (result & 0xFF0000) >> 16;
    dev.latencyTimer = (result & 0xFF00) >> 8;
    dev.cacheLineSize = (result & 0xFF);
    
    request = build_request(bus, device, function, 0x10);
    outdw(PCI_INDEX_PORT, request);
    result = indw(PCI_DATA_PORT);
    
    dev.barAddress0 = result;
    
    request = build_request(bus, device, function, 0x14);
    outdw(PCI_INDEX_PORT, request);
    result = indw(PCI_DATA_PORT);
    
    dev.barAddress1 = result;
    
    request = build_request(bus, device, function, 0x18);
    outdw(PCI_INDEX_PORT, request);
    result = indw(PCI_DATA_PORT);
    
    dev.barAddress2 = result;
    
    request = build_request(bus, device, function, 0x1C);
    outdw(PCI_INDEX_PORT, request);
    result = indw(PCI_DATA_PORT);
    
    dev.barAddress3 = result;
    
    request = build_request(bus, device, function, 0x20);
    outdw(PCI_INDEX_PORT, request);
    result = indw(PCI_DATA_PORT);
    
    dev.barAddress4 = result;
    
    request = build_request(bus, device, function, 0x24);
    outdw(PCI_INDEX_PORT, request);
    result = indw(PCI_DATA_PORT);
    
    dev.barAddress5 = result;
    
    request = build_request(bus, device, function, 0x28);
    outdw(PCI_INDEX_PORT, request);
    result = indw(PCI_DATA_PORT);
    
    dev.cardbusCISPointer = result;
    
    request = build_request(bus, device, function, 0x2C);
    outdw(PCI_INDEX_PORT, request);
    result = indw(PCI_DATA_PORT);
    
    dev.subsystemID = result >> 16;
    dev.subsystemVendorID = result & 0xFFFF;
    
    request = build_request(bus, device, function, 0x30);
    outdw(PCI_INDEX_PORT, request);
    result = indw(PCI_DATA_PORT);
    
    dev.expansionROMBaseAddress = result;
    
    request = build_request(bus, device, function, 0x34);
    outdw(PCI_INDEX_PORT, request);
    result = indw(PCI_DATA_PORT);
    
    dev.reservedOne = 0;
    dev.reservedTwo = 0;
    dev.capabilitiesPointer = result & 0xFF;
    
    dev.reservedThree = 0;
    
    request = build_request(bus, device, function, 0x3C);
    outdw(PCI_INDEX_PORT, request);
    result = indw(PCI_DATA_PORT);
    
    dev.maxLatency = result >> 24;
    dev.minGrant = (result & 0xFF0000) >> 16;
    dev.intPin = (result & 0xFF00) >> 8;
    dev.intLine = (result & 0xFF);
    
    return dev;
}

struct pci_device** pci_scan_bus(int* count)
{
    struct vector* vec = vector_create();
    
    int busCount = 256;
    int deviceCount = 32;
    int functionCount = 8;
    
    for(int b = 0; b < busCount; b++)
    {
        for(int d = 0; d < deviceCount; d++)
        {
            for(int f = 0; f < functionCount; f++)
            {
                struct pci_device dev = get_device(b, d, f);
                if(dev.valid_device)
                {
                    struct pci_device* heapDevice = malloc(sizeof(struct pci_device));
                    memcpy(heapDevice, &dev, sizeof(struct pci_device));
                    
                    vector_add(vec, heapDevice);
                }
            }
        }
    }
    
    struct pci_device** array = (struct pci_device**)vector_get_array(vec, count);
    
    free(vec);
    
    return array;
}

struct pci_controlset* get_devices_list(int* count)
{
    struct pci_controlset* cs = malloc(sizeof(struct pci_controlset));
    
    int scanCount = 0;
    cs->deviceList = pci_scan_bus(&scanCount);
    cs->devicesCount = scanCount;
    
    *count = scanCount;
    
    return cs;
}

uint32_t build_request(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg)
{
    uint32_t result = 0;
    uint32_t dwBus = (uint32_t)bus;
    uint32_t dwDevice = (uint32_t)device & 0x1F;
    uint32_t dwFunction = (uint32_t)function & 0x7;
    uint32_t dwReg = (uint32_t)reg & 0xFC;
    
    result = 0x80000000 | (dwBus << 16) | (dwDevice << 11) | (dwFunction << 8) | (dwReg);
    
    return result;
}

void print_pci_device_info(struct pci_device* device)
{
    kWriteLog("********************");
    kWriteLog("PCI Device Info");
    kWriteLog("                    ");
    kWriteLog("Bus : %d", device->bus);
    kWriteLog("Device : %d", device->device);
    kWriteLog("Function : %d", device->function);
    kWriteLog("                    ");
    kWriteLog("Device ID : %d", device->deviceID);
    kWriteLog("Vendor ID : %d", device->vendorID);
    kWriteLog("                    ");
    kWriteLog("Status : %d", device->status);
    kWriteLog("Command : %d", device->command);
    kWriteLog("                    ");
    kWriteLog("Class code : %d (%s)", device->classCode, pci_class_name(device->classCode));
    kWriteLog("Sub class : %d (%s)", device->subClass, pci_subclass_name(device->classCode, device->subClass));
    kWriteLog("Prog IF : %d", device->progIF);
    kWriteLog("Rev ID : %d", device->revID);
    kWriteLog("                    ");
    kWriteLog("BIST : %d", device->BIST);
    kWriteLog("Header type : %d", device->headerType);
    kWriteLog("Latency timer : %d", device->latencyTimer);
    kWriteLog("Cache line size : %d", device->cacheLineSize);
    kWriteLog("                    ");
    kWriteLog("BAR Address #0 : %d", device->barAddress0);
    kWriteLog("BAR Address #1 : %d", device->barAddress1);
    kWriteLog("BAR Address #2 : %d", device->barAddress2);
    kWriteLog("BAR Address #3 : %d", device->barAddress3);
    kWriteLog("BAR Address #4 : %d", device->barAddress4);
    kWriteLog("BAR Address #5 : %d", device->barAddress5);
    kWriteLog("                    ");
    kWriteLog("Cardbus CIS Pointer : %d", device->cardbusCISPointer);
    kWriteLog("                    ");
    kWriteLog("Subsystem ID : %d", device->subsystemID);
    kWriteLog("Subsystem Vendor ID : %d", device->subsystemVendorID);
    kWriteLog("                    ");
    kWriteLog("Expansion ROM base address : %d", device->expansionROMBaseAddress);
    kWriteLog("Capabilities pointer : %d", device->capabilitiesPointer);
    kWriteLog("                    ");
    kWriteLog("Max latency : %d", device->maxLatency);
    kWriteLog("Min grant : %d", device->minGrant);
    kWriteLog("Interrupt Pin : %d", device->intPin);
    kWriteLog("Interrupt Line: %d", device->intLine);
    kWriteLog("                    ");
    kWriteLog("********************");
}

const char* pci_class_name(uint8_t pci_class)
{
    switch(pci_class)
    {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
        case 0x08:
        case 0x09:
        case 0x0a:
        case 0x0b:
        case 0x0c:
        case 0x0d:
        case 0x0e:
        case 0x0f:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x40:
        case 0xff:
        {
            return classnames[pci_class];
        }
        default:
        {
            return "Class not found";
        }
    }
}

const char* pci_subclass_name(uint8_t pci_class, uint8_t subclass)
{
    if(subclass == 128)
    {
        return "Generic";
    }
    
    switch(pci_class)
    {
        case 0x0:
        {
            return "Unclassed";
        }
        case 0x01:
        {
            return massstoragenames[subclass];
        }
        case 0x02:
        {
            return netcontrollernames[subclass];
        }
        case 0x03:
        {
            return displaynames[subclass];
        }
        case 0x04:
        {
            return multimediacontrollernames[subclass];
        }
        case 0x05:
        {
            return memorynames[subclass];
        }
        case 0x06:
        {
            return bridgenames[subclass];
        }
        case 0x07:
        {
            return communicationnames[subclass];
        }
        case 0x08:
        {
            return systemnames[subclass];
        }
        case 0x09:
        {
            return inputnames[subclass];
        }
        case 0xc:
        {
            return serialbusnames[subclass];
        }
        default:
        {
            return "Class not found.";
        }
    }
}

