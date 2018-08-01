#include "ahci_driver.h"

#include "pci.h"
#include "memory.h"
#include "flagutils.h"

void init_module_ahci_driver(struct kernel_info_block* kinfo)
{
    ahci_driver = alloc_kernel_module(sizeof(struct ahci_driver_info));
    kinfo->m_ahci_driver = ahci_driver;

    ahci_driver->port_count = 0;
    memset(ahci_driver->disk_ports, 0, 32);
    ahci_driver->hba_device = NULL;
}

int driver_ahci_find_disks(struct pci_controlset* pcs)
{
    struct pci_device* dev = NULL;
    for(int i = 0; i < pcs->devicesCount; i++)
    {
        if(pcs->deviceList[i]->classCode == PCI_CLA_MASS_STORAGE &&
           pcs->deviceList[i]->subClass == PCI_SUB_MASS_STOR_SATA_CONTROLLER )
           {
               // Multiple SATA HBA not supported.
               dev = pcs->deviceList[i];
               break;
           }
    }

    // If we found a SATA host bus adapter
    if(dev != NULL)
    {
        ahci_driver->hba_device = dev;
        ahci_driver->abar = dev->barAddress5;

        // Fill the driver fields with ports information.
        int res = driver_ahci_get_disk_ports(ahci_driver->disk_ports, 
                                             &ahci_driver->port_count);
        
        if(FAILED(res))
        {
            return res;
        }
    }

    return E_OK;
}

uint8_t driver_ahci_get_default_port()
{
    if(ahci_driver->port_count > 0)
    {
        return ahci_driver->disk_ports[ahci_driver->port_count];
    }
    
    return AHCI_INVALID_PORT;
}

int driver_ahci_read_GHC_regs(struct ahci_host_regs* regs)
{
    memcpy(regs, (const void*)ahci_driver->abar, sizeof(struct ahci_host_regs));

    return E_OK;
}

int driver_ahci_read_port_regs(int portNb, struct ahci_port_regs* regs)
{
    uint32_t portAddress = ahci_driver->abar + 0x100 + (portNb * 0x80);

    memcpy(regs, (void*)portAddress, sizeof(struct ahci_port_regs));

    return E_OK;
}

int driver_ahci_read_port_commandlist(int portNb, struct ahci_port_commandlist* data)
{
    uint32_t portAddress = ahci_driver->abar + 0x100 + (portNb * 0x80);

    struct ahci_port_regs* port = (struct ahci_port_regs*)portAddress;

    uint32_t cmdlistaddr = port->command_list_base_addr_lower;

    memcpy(data, (struct ahci_port_commandlist*)cmdlistaddr, sizeof(struct ahci_port_commandlist));

    return E_OK;
}

int driver_ahci_read_port_commandtable(int portNb, int commandNb, struct ahci_port_commandtable* data)
{
    (void)portNb;
    (void)commandNb;
    (void)data;
    
    return E_OK;
}

int driver_ahci_get_ports_enabled()
{
    struct ahci_host_regs target_hba;
    driver_ahci_read_GHC_regs(&target_hba);

    int np = AHCI_CAP_NP(target_hba.host_capabilities);

    return np + 1; // Add 1 because its 0 based count. 0 equals 1 port available
}

int driver_ahci_get_disk_ports(uint8_t* ports, int* amount)
{
    // Get the GHC regs.
    struct ahci_host_regs target_hba;
    driver_ahci_read_GHC_regs(&target_hba);

    // Each bit set in the PI byte represent a particular port being open.
    // Need to go through each bit and check its its one then add it in the list
    uint32_t portsImplemented = target_hba.ports_implemented;

    // Next position in the 'ports' parameter to add a found port.
    size_t portsIndex = 0;

    // Set the current
    for(int i = 0; i < 32; i++)
    {
        uint32_t res = 0x1 << i;

        if(FLAG(portsImplemented, res))
        {
            ports[portsIndex] = i;
            portsIndex++;
        }
    }

    *amount = portsIndex;
    
    return E_OK;
}

int driver_ahci_print_ports_info()
{
    for(int i = 0; i < ahci_driver->port_count; i++)
    {
        struct ahci_port_regs pregs;
        int read_res = driver_ahci_read_port_regs(i, &pregs);
        
        if(SUCCESS(read_res))
        {
            kWriteLog("*** AHCI Port %d Registers START ***", i);
            kWriteLog("command_list_base_addr_lower: %d", pregs.command_list_base_addr_lower);
            kWriteLog("command_list_base_addr_upper: %d", pregs.command_list_base_addr_upper);
            kWriteLog("fis_base_addr_lower: %d", pregs.fis_base_addr_lower);
            kWriteLog("fis_base_addr_upper: %d", pregs.fis_base_addr_upper);
            kWriteLog("interrupt_status: %d", pregs.interrupt_status);
            kWriteLog("interrupt_enable: %d", pregs.interrupt_enable);
            kWriteLog("command_and_status: %d", pregs.command_and_status);
            kWriteLog("reserved1: %d", pregs.reserved1);
            kWriteLog("task_file_data: %d", pregs.task_file_data);
            kWriteLog("signature: %d", pregs.signature);
            kWriteLog("serial_ata_status: %d", pregs.serial_ata_status);
            kWriteLog("serial_ata_control: %d", pregs.serial_ata_control);
            kWriteLog("serial_ata_error: %d", pregs.serial_ata_error);
            kWriteLog("serial_ata_active: %d", pregs.serial_ata_active);
            kWriteLog("serial_command_issue: %d", pregs.serial_command_issue);
            kWriteLog("serial_ata_notification: %d", pregs.serial_ata_notification);
            kWriteLog("fis_based_switching_control: %d", pregs.fis_based_switching_control);
            kWriteLog("device_sleep: %d", pregs.device_sleep);
            kWriteLog("reserved2: %d", pregs.reserved2);
            kWriteLog("vendor_specific: %d", pregs.vendor_specific);
            kWriteLog("*** AHCI Port %d Registers END ***\n\n", i);
        }
        else
        {
            return E_UNKNOWN;
        }
    }
    
    return E_OK;
}