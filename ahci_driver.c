#include "ahci_driver.h"

#include "pci.h"
#include "memory.h"
#include "flagutils.h"
#include "status.h"

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
        if(dev->barAddress5 == 0)
        {
            // We found an invalid device and it somehow had the correct bits
            // report error.
            return E_DEVICE_INVALID;
        }

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
    else
    {
        return E_NODISK;
    }

    return E_OK;
}

int driver_ahci_setup_memory(uint8_t portNb)
{
    // TODO : Find area of memory suitable for ahci data.
    uint32_t ahci_memory_zone_addr = 1024*1024*10;

    struct ahci_port_regs* regs;
    int res = driver_ahci_get_port_regs(portNb, &regs);
    if(FAILED(res))
        return res;

    // Stop command engine
    regs->command_and_status &= ~0x0001;
    while(TRUE)
    {
        if(AHCI_PxCMD_ST(regs->command_and_status))
            continue;
        if(AHCI_PxCMD_CR(regs->command_and_status))
            continue;

        break;
    }
    regs->command_and_status &= ~0x0010;

    regs->command_list_base_addr_lower = ahci_memory_zone_addr + (portNb << 10);
    regs->command_list_base_addr_upper = 0;
    memset((void*)regs->command_list_base_addr_lower, 0, 1024);

    regs->fis_base_addr_lower = ahci_memory_zone_addr + (32<<10) + (portNb << 8);
    regs->fis_base_addr_upper = 0;
    memset((void*)regs->fis_base_addr_lower, 0, 256);

    struct ahci_port_commandlist* commands = (struct ahci_port_commandlist*)regs->command_list_base_addr_lower;
    for(int i = 0; i < 32; i++)
    {
        commands->entries[i].prdtl = 8;
        commands->entries[i].commandtableBaseAddr = ahci_memory_zone_addr + (40 << 10) + (portNb << 13) + (i << 8);
        commands->entries[i].commandtableBaseAddrUpper = 0;
        memset((void*)commands->entries[i].commandtableBaseAddr, 0, 256);
    }

    // Start command engine
    while(AHCI_PxCMD_CR(regs->command_and_status));

    regs->command_and_status |= 0x0010;
    regs->command_and_status |= 0x0001;

    return E_OK;
}

uint8_t driver_ahci_get_default_port()
{
    if(ahci_driver->port_count > 0)
    {
        for(int i = 0; i < ahci_driver->port_count; i++)
        {
            struct ahci_port_regs portInfo;
            int res = driver_ahci_read_port_regs(i, &portInfo);
            if(FAILED(res))
            {
                kWriteLog("Unable to get the registers of port %d : %d", i, res);
                continue;
            }

            if(portInfo.signature == AHCI_SIG_ATA)
            {
                return ahci_driver->disk_ports[i];
            }
        }
    }

    return AHCI_INVALID_PORT;
}

int driver_ahci_read_GHC_regs(struct ahci_host_regs* regs)
{
    memcpy(regs, (const void*)ahci_driver->abar, sizeof(struct ahci_host_regs));

    return E_OK;
}

int driver_ahci_read_port_regs(uint8_t portNb, struct ahci_port_regs* regs)
{
    // TODO : Check port isn't a nonsense index.

    uint32_t portAddress = ahci_driver->abar + 0x100 + (portNb * 0x80);

    memcpy(regs, (void*)portAddress, sizeof(struct ahci_port_regs));

    return E_OK;
}

int driver_ahci_read_port_commandlist(uint8_t portNb, struct ahci_port_commandlist* data)
{
    uint32_t portAddress = ahci_driver->abar + 0x100 + (portNb * 0x80);

    struct ahci_port_regs* port = (struct ahci_port_regs*)portAddress;

    uint32_t cmdlistaddr = port->command_list_base_addr_lower;

    memcpy(data, (struct ahci_port_commandlist*)cmdlistaddr, sizeof(struct ahci_port_commandlist));

    return E_OK;
}

int driver_ahci_read_port_commandtable(uint8_t portNb, int commandNb, struct ahci_port_commandtable* data)
{
    (void)portNb;
    (void)commandNb;
    (void)data;

    return E_OK;
}

int driver_ahci_get_GHC_regs(struct ahci_host_regs** regs)
{
    *regs = (struct ahci_host_regs*)ahci_driver->abar;

    return E_OK;
}

int driver_ahci_get_port_regs(uint8_t portNb, struct ahci_port_regs** regs)
{
    uint32_t portAddress = ahci_driver->abar + 0x100 + (portNb * 0x80);

    *regs = (struct ahci_port_regs*)portAddress;

    return E_OK;
}

int driver_ahci_get_port_commandlist(uint8_t portNb, struct ahci_port_commandlist** data)
{
    uint32_t portAddress = ahci_driver->abar + 0x100 + (portNb * 0x80);

    struct ahci_port_regs* port = (struct ahci_port_regs*)portAddress;

    uint32_t cmdlistaddr = port->command_list_base_addr_lower;

    *data = (struct ahci_port_commandlist*)cmdlistaddr;

    return E_OK;
}

int driver_ahci_get_port_commandtable(uint8_t portNb, int commandNb, struct ahci_port_commandtable** data)
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

int driver_ahci_get_disk_ports(uint8_t* ports, uint8_t* amount)
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

int driver_ahci_reset_controller()
{
    struct ahci_host_regs* host;
    int res = driver_ahci_get_GHC_regs(&host);

    if(SUCCESS(res))
    {
        // Set the reset bit to 1
        host->global_host_control |= 0x1;

        // Poll the same bit until it goes back to 0
        //
        int loopCount = 0;
        while(TRUE)
        {
            if(loopCount > 1000)
            {
                // HBA has max 1 second to reset
                return E_IO_TIMEOUT;
            }

            // Bit set to 0 when finished.
            if(FLAG(host->global_host_control, 0x1) == FALSE )
            {
                break;
            }
            else
            {
                sleep(1);
            }

            loopCount++;
        }

        // TODO : Check if we support Staggered Spin Up
        // That means we have to restart each disk one by
        // one ourselves.

        return E_OK;
    }

    return res;
}

int driver_ahci_reset_port(uint8_t portNb)
{
    struct ahci_port_regs* regs;
    int res = driver_ahci_get_port_regs(portNb, &regs);
    if(SUCCESS(res))
    {
        regs->command_and_status &= ~0x1;

        int loopCount = 0;
        while(TRUE)
        {
            if(loopCount > 1000)
            {
                return E_IO_TIMEOUT;
            }

            // Wait until the port stops running the current command, if any.
            if(regs->serial_command_issue == 0)
            {
                break;
            }
            else
            {
                sleep(1);
            }

            loopCount++;
        }

        return E_OK;
    }

    return res;
}

int driver_ahci_reset_all_ports()
{
    uint8_t portCount = 0;
    uint8_t* enabledPorts = NULL;
    int res = driver_ahci_get_disk_ports(enabledPorts, &portCount);
    if(SUCCESS(res))
    {
        for(uint8_t i = 0; i < portCount; i++)
        {
            res = driver_ahci_reset_port(enabledPorts[i]);

            if(FAILED(res))
            {
                kWriteLog("Port %d failed to reset", i);
                return res;
            }
        }

        return E_OK;
    }

    return E_DEVICE_ERROR;
}

int driver_ahci_read_data(uint8_t port, uint32_t addr_low, uint32_t addr_high, uint32_t length, uint8_t* buf)
{
    uint8_t cmdSlot = 0;
    int res = driver_ahci_get_next_cmdslot(port, &cmdSlot);

    if(res == E_IO_FULL)
    {
        // Wait a bit for IO to commands to free up
    }
    else if(FAILED(res))
    {
        return res;
    }

    struct ahci_port_command_header* command = NULL;
    memset(command, 0, sizeof(struct ahci_port_command_header));
    res = driver_ahci_make_command_header(port, cmdSlot, &command);

    if(FAILED(res))
        return res;

    command->prflags = sizeof(struct ahci_fis_reg_H2D) / sizeof(uint32_t); // 5
    command->prflags = command->prflags & 0xFFBF; // Take all bits except #6, make sure #6 is 0 for a Read operation

    // Need to calculate how many PRDT entries we will need
    // Need to know how much data can a PRDT contain.
    command->prdtl = 1; // Amount of PRDT entries in the table

    // For now, we'll just use one PRDT
    struct ahci_port_commandtable* table = (struct ahci_port_commandtable*)command->commandtableBaseAddr;
    table->regions[0].addr_base = (uint32_t)buf;
    table->regions[0].bytecount = length; // 512 bytes per sector
    table->regions[0].bytecount |= 0x80000000; // Set bit #31 to enable interrupt on completion

    // TODO : Fill the table PRDT regions

    struct ahci_fis_reg_H2D* cmd_fis = NULL;
    res = driver_ahci_make_command_fis(table, &cmd_fis);
    cmd_fis->type = AHCI_FIS_REG_H2D;
    cmd_fis->commandreg = 1;
    cmd_fis->command = ATA_CMD_READ_SECTORS; // ATA_CMD_READ_DMA_EX
    cmd_fis->device = 1<<6; // LBA Mode

    cmd_fis->lba_1 = (uint8_t)addr_low;
    cmd_fis->lba_2 = (uint8_t)(addr_low >> 8);
    cmd_fis->lba_3 = (uint8_t)(addr_low >> 16);
    cmd_fis->lba_4 = (uint8_t)(addr_low >> 24);
    cmd_fis->lba_5 = (uint8_t)(addr_high);
    cmd_fis->lba_6 = (uint8_t)(addr_high >> 8);

    cmd_fis->count_low = length && 0xFF;
    cmd_fis->count_high = (length >> 8) & 0xFF;

    // Wait until drive is ready for requests
    int waitloop = 0;
    BOOL waiting = TRUE;
    struct ahci_port_regs* regs = NULL;
    res = driver_ahci_get_port_regs(port,  &regs);
    while(waiting)
    {
        if(regs->task_file_data & (ATA_STATUS_BUSY | ATA_STATUS_DATA_REQUEST))
        {
            waitloop++;
        }
        else
        {
            break;
        }

        if(waitloop > 10000)
        {
            kWriteLog("IO TIMEOUT");

            return E_IO_TIMEOUT;
        }
    }

    regs->serial_command_issue = 1 << port; // Start the IO operation

    waitloop = 0;
    waiting = TRUE;
    while(waiting)
    {
        if(waitloop > 1000)
        {
            return E_IO_TIMEOUT;
        }

        if((regs->serial_command_issue & (1 << port)) == 0)
        {
            break; // Command has cleared !
        }
        if(regs->interrupt_status & 0x40000000)
        {
            kWriteLog("IO Error");
            return E_IO_ERROR;
        }

        sleep(1);

        waitloop++;
    }

    return E_OK;
}

int driver_ahci_identify(uint8_t port, struct ata_identify_device* data)
{
    uint8_t cmdSlot = 0;
    int res = driver_ahci_get_next_cmdslot(port, &cmdSlot);

    if(res == E_IO_FULL)
    {
        // Wait a bit for IO to commands to free up
    }
    else if(FAILED(res))
    {
        return res;
    }

    struct ahci_port_command_header* command = NULL;
    memset(command, 0, sizeof(struct ahci_port_command_header));
    res = driver_ahci_make_command_header(port, cmdSlot, &command);

    if(FAILED(res))
        return res;

    command->prflags = sizeof(struct ahci_fis_reg_H2D) / sizeof(uint32_t); // 5
    command->prflags = command->prflags & 0xFFBF; // Take all bits except #6, make sure #6 is 0 for a Read operation

    // Need to calculate how many PRDT entries we will need
    // Need to know how much data can a PRDT contain.
    command->prdtl = 1; // Amount of PRDT entries in the table

    size_t dataSize = sizeof(struct ata_identify_device);

    // For now, we'll just use one PRDT
    struct ahci_port_commandtable* table = (struct ahci_port_commandtable*)command->commandtableBaseAddr;
    table->regions[0].addr_base = (uint32_t)data;
    table->regions[0].bytecount = dataSize;
    table->regions[0].bytecount |= 0x80000000; // Set bit #31 to enable interrupt on completion

    // TODO : Fill the table PRDT regions

    struct ahci_fis_reg_H2D* cmd_fis = NULL;
    res = driver_ahci_make_command_fis(table, &cmd_fis);
    cmd_fis->type = AHCI_FIS_REG_H2D;
    cmd_fis->commandreg = 1;
    cmd_fis->command = ATA_CMD_IDENTIFY_DEVICE; // ATA_CMD_READ_DMA_EX
    cmd_fis->device = 1<<6; // LBA Mode

    cmd_fis->lba_1 = 0;
    cmd_fis->lba_2 = 0;
    cmd_fis->lba_3 = 0;
    cmd_fis->lba_4 = 0;
    cmd_fis->lba_5 = 0;
    cmd_fis->lba_6 = 0;

    cmd_fis->count_low = dataSize && 0xFF;
    cmd_fis->count_high = (dataSize >> 8) & 0xFF;

    // Wait until drive is ready for requests
    int waitloop = 0;
    BOOL waiting = TRUE;
    struct ahci_port_regs* regs = NULL;
    res = driver_ahci_get_port_regs(port,  &regs);
    while(waiting)
    {
        if(regs->task_file_data & (ATA_STATUS_BUSY | ATA_STATUS_DATA_REQUEST))
        {
            waitloop++;
        }
        else
        {
            break;
        }

        if(waitloop > 10000)
        {
            kWriteLog("IO TIMEOUT");

            return E_IO_TIMEOUT;
        }
    }

    regs->serial_command_issue = 1 << port; // Start the IO operation

    waitloop = 0;
    waiting = TRUE;
    while(waiting)
    {
        if(waitloop > 1000)
        {
            return E_IO_TIMEOUT;
        }

        if((regs->serial_command_issue & (1 << port)) == 0)
        {
            break; // Command has cleared !
        }
        if(regs->interrupt_status & 0x40000000)
        {
            kWriteLog("IO Error");
            return E_IO_ERROR;
        }

        sleep(1);

        waitloop++;
    }

    return E_OK;
}

int driver_ahci_make_command_header(uint8_t portNb, uint8_t cmdslot, struct ahci_port_command_header** cmd)
{
    struct ahci_port_regs pregs;
    driver_ahci_read_port_regs(portNb, &pregs);
    *cmd = (struct ahci_port_command_header*)pregs.command_list_base_addr_lower;
    *cmd += cmdslot;

    return E_OK;
}

int driver_ahci_make_command_fis(struct ahci_port_commandtable* cmdtable, struct ahci_fis_reg_H2D** fis)
{
    *fis = (struct ahci_fis_reg_H2D*)&cmdtable->cmd_fis;

    return E_OK;
}

int driver_ahci_get_next_cmdslot(uint8_t portNb, uint8_t* cmdslot)
{
    struct ahci_host_regs hregs;
    int res = driver_ahci_read_GHC_regs(&hregs);

    struct ahci_port_regs pregs;
    res = driver_ahci_read_port_regs(portNb, &pregs);

    if(FAILED(res))
        return res;

    // One of these ports have bit N set when the N command is running.
    uint32_t slotstatus = pregs.serial_ata_active | pregs.serial_command_issue;

    int cmdSlotsAvailable = AHCI_CAP_NCS(hregs.host_capabilities);
    // Go through each bit and check if one is not set, that port will be free.
    // If set, shift bits to the right for the next evaluation.
    // Checking the rightmost bit and shifting is just an easy way to check each
    // bit.
    for(uint8_t i = 0; i < cmdSlotsAvailable; i++)
    {
        if((slotstatus & 1) == 0)
        {
            *cmdslot = i;

            return E_OK;
        }
        else
        {
            slotstatus = slotstatus >> 1;
        }
    }

    // No ports are free, return some bad value and flag an error.
    *cmdslot = 255;

    return E_IO_FULL;
}
