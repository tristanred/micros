#include "ahci_driver.h"

#include "pci.h"
#include "memory.h"
#include "flagutils.h"

uint32_t driver_ahci_get_default_disk()
{
    int total = 0;
    struct pci_controlset* set = get_devices_list(&total);
    
    struct pci_device* dev = NULL;
    for(int i = 0; i < total; i++)
    {
        if(set->deviceList[i]->classCode == PCI_CLA_MASS_STORAGE &&
           set->deviceList[i]->subClass == PCI_SUB_MASS_STOR_SATA_CONTROLLER )
           {
               dev = set->deviceList[i];
           }
    }

    if(dev != NULL)
    {
        uint32_t abar = dev->barAddress5;
        
        return abar;
    }
    
    return 0;
}

int driver_ahci_read_GHC_regs(uint32_t abar, struct ahci_generic_host_regs* regs)
{
    memcpy(regs, (const void*)abar, sizeof(struct ahci_generic_host_regs));
    
    return 0;
}

int driver_ahci_read_port_regs(uint32_t abar, int portNb, struct ahci_port_regs* regs)
{
    uint32_t portAddress = abar + 0x100 + (portNb * 0x80);
    
    memcpy(regs, (void*)portAddress, sizeof(struct ahci_port_regs));
    
    return 0;
}

int driver_ahci_read_port_commandlist(uint32_t abar, int portNb, struct ahci_port_commandlist* data)
{
    uint32_t portAddress = abar + 0x100 + (portNb * 0x80);
    
    struct ahci_port_regs* port = (struct ahci_port_regs*)portAddress;
    
    uint32_t cmdlistaddr = port->command_list_base_addr_lower;
    
    memcpy(data, (struct ahci_port_commandlist*)cmdlistaddr, sizeof(struct ahci_port_commandlist));

    return 0;
}

int driver_ahci_read_port_commandtable(uint32_t abar, int portNb, int commandNb, struct ahci_port_commandtable* data)
{
    return 0;
}

int driver_ahci_get_ports_enabled(uint32_t abar)
{
    struct ahci_generic_host_regs target_hba;
    driver_ahci_read_GHC_regs(abar, &target_hba);
    
    int np = AHCI_CAP_NP(target_hba.host_capabilities);
    
    return np + 1; // Add 1 because its 0 based count. 0 equals 1 port available
}

int driver_ahci_get_disk_ports(uint32_t abar, int* ports, size_t* amount)
{
    // Get the GHC regs.
    struct ahci_generic_host_regs target_hba;
    driver_ahci_read_GHC_regs(abar, &target_hba);
    
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
}
