#include "ahci_terminal.h"

#include "memory.h"

void init_ahci_term()
{
    // display = (char**)kmalloc(FBROWS * sizeof(char*));
    
    // for(int i = 0; i < FBROWS; i++)
    // {
    //     display[i] = (char*)kmalloc(FBCOLS * sizeof(char));
    // }
    
    // displayWidth = FBCOLS;
    // displayHeight = FBROWS;
    
    // for(int i = 0; i < displayHeight; i++)
    // {
    //     for(int k = 0; k < displayWidth; k++)
    //     {
    //         display[i][k] = ' ';
    //     }
    // }
    
    RegisterKeyboardHook(&ahci_term_kbhook);
    
    current_state = MAIN_SCREEN;
    view_port_nb = 0;
    view_command_nb = 0;
}

void ahci_term_update()
{
    ahci_term_drawoverlay();
    
    // Get the host regs (11 dwords)
    struct ahci_host_regs host;
    int res = driver_ahci_read_GHC_regs(&host);
    if(FAILED(res))
        goto error;
    
    char buf[256];
    
    if(current_state == MAIN_SCREEN)
    {
        fbMoveCursor(17, 4);
        sprintf(buf, "%d", host.global_host_control);
        fbPutString(buf);
        
        fbMoveCursor(17, 5);
        sprintf(buf, "%d", host.global_host_control);
        fbPutString(buf);
        
        fbMoveCursor(17, 6);
        sprintf(buf, "%d", host.interrupt_status);
        fbPutString(buf);
        
        fbMoveCursor(17, 7);
        sprintf(buf, "%d", host.ports_implemented);
        fbPutString(buf);
        
        fbMoveCursor(17, 8);
        sprintf(buf, "%d", host.version);
        fbPutString(buf);
        
        fbMoveCursor(17, 9);
        sprintf(buf, "%d", host.command_completion_coalescing_control);
        fbPutString(buf);
        
        fbMoveCursor(17, 10);
        sprintf(buf, "%d", host.command_completion_coalescing_ports);
        fbPutString(buf);
        
        fbMoveCursor(17, 11);
        sprintf(buf, "%d", host.enclosure_management_location);
        fbPutString(buf);
        
        fbMoveCursor(17, 12);
        sprintf(buf, "%d", host.enclosure_management_control);
        fbPutString(buf);
        
        fbMoveCursor(17, 13);
        sprintf(buf, "%d", host.host_capabilities_extended);
        fbPutString(buf);
        
        fbMoveCursor(17, 14);
        sprintf(buf, "%d", host.bios_handoff_control_status);
        fbPutString(buf);
        
        uint8_t ports[32] = { 0 };
        uint8_t portNb = 0;
        res = driver_ahci_get_disk_ports(ports, &portNb);
        if(FAILED(res))
            goto error;
        
        for(uint8_t i = 0; i < portNb; i++)
        {
            struct ahci_port_regs portregs;
            res = driver_ahci_read_port_regs(ports[i], &portregs);
            if(FAILED(res))
                goto error;
            
            fbMoveCursor(35, 5 + i);
            sprintf(buf, "%d", portregs.command_and_status);
            fbPutString(buf);
            
            fbMoveCursor(47, 5 + i);
            sprintf(buf, "%d", portregs.interrupt_status);
            fbPutString(buf);

            fbMoveCursor(59, 5 + i);
            sprintf(buf, "%d", portregs.serial_ata_status);
            fbPutString(buf);
            
            fbMoveCursor(72, 5 + i);
            if(portregs.signature == 0x00000101)
            {
                sprintf(buf, "%s", "ATA");
            }
            else if(portregs.signature == 0xEB140101)
            {
                sprintf(buf, "%s", "ATAPI");
            }
            else if(portregs.signature == 0xC33C0101)
            {
                sprintf(buf, "%s", "SEMB");
            }
            else if(portregs.signature == 0x96690101)
            {
                sprintf(buf, "%s", "PM");
            }
            else
            {
                sprintf(buf, "%s", "UKNOWN");
            }
            
            fbPutString(buf);
        }
    }
    else if(current_state == PORT_SCREEN)
    {
        
    }
    else if(current_state == COMMAND_SCREEN)
    {
        
    }
    
    // Get the enabled ports regs 20 dwords
    return;
error:
    kWriteLog("AHCI Error %d", res);
}

void ahci_term_task()
{
    while(TRUE)
    {
        ahci_term_update();
        sleep(1);
    }
}

void ahci_term_kbhook(keyevent_info* info)
{
    
}

void ahci_term_drawoverlay()
{
    switch(current_state)
    {
        case MAIN_SCREEN:
        {
            ahci_term_drawoverlay_main();
            break;
        }
        case PORT_SCREEN:
        {
            ahci_term_drawoverlay_port();
            break;
        }
        case COMMAND_SCREEN:
        {
            ahci_term_drawoverlay_command();
            break;
        }
    }
}

void ahci_term_drawoverlay_main()
{
    fbMoveCursor(0, 0);
    fbPutString(" +-------------------------+ +-------------------------------------------------+");
    fbPutString(" |   Generic Host Regs     | |             Ports Status                        |");
    fbPutString(" +-------------------------+ |      CMD & STAT    INT STAT    SATA      Type   |");
    fbPutString(" |Host Cap     =           | +-------------------------------------------------+");
    fbPutString(" |GBL Host Ctl =           | | 0 =                                             |");
    fbPutString(" |Interrupt St =           | | 1 =                                             |");
    fbPutString(" |Ports Impl   =           | | 2 =                                             |");
    fbPutString(" |Version      =           | | 3 =                                             |");
    fbPutString(" |CCC  CTL     =           | | 4 =                                             |");
    fbPutString(" |CCC Ports    =           | | 5 =                                             |");
    fbPutString(" |EM  Location =           | | 6 =                                             |");
    fbPutString(" |EM  CTL      =           | | 7 =                                             |");
    fbPutString(" |Host Cap Ext =           | | 8 =                                             |");
    fbPutString(" |Bios Handoff =           | | 9 =                                             |");
    fbPutString(" +-------------------------+ +-------------------------------------------------+");
    fbPutString("                                                                                ");
    fbPutString("                                                                                ");
    fbPutString("                                                                                ");
    fbPutString("                                                                                ");
    fbPutString("                                                                                ");
}

void ahci_term_drawoverlay_port()
{
    fbMoveCursor(0, 0);
    fbPutString(" +-----------------+ +--------------------------------------------+             ");
    fbPutString(" | Port Regs       | | Active CMDs = (32) : 0 1 2 3 4 5 6 7 8 9   |             ");
    fbPutString(" +-----------------+ +--------------------------------------------+             ");
    fbPutString(" |CLB  =           | +-----------------------------+                            ");
    fbPutString(" |CLBU =           | | CMDs                        |                            ");
    fbPutString(" |FB   =           | +-----------------------------+                            ");
    fbPutString(" |FBU  =           | |0.PRDTL        6.PRDTL       |                            ");
    fbPutString(" |IS   =           | |0.PFLAG        6.PFLAG       |                            ");
    fbPutString(" |IE   =           | |0.PBC          6.PBC         |                            ");
    fbPutString(" |CMD  =           | |1.PRDTL        7.PRDTL       |                            ");
    fbPutString(" |TFD  =           | |1.PFLAG        7.PFLAG       |                            ");
    fbPutString(" |SIG  =           | |1.PBC          7.PBC         |                            ");
    fbPutString(" |SSTS =           | |2.PRDTL        8.PRDTL       |                            ");
    fbPutString(" |SCTL =           | |2.PFLAG        8.PFLAG       |                            ");
    fbPutString(" |SERR =           | |2.PBC          8.PBC         |                            ");
    fbPutString(" |SACT =           | |3.PRDTL        9.PRDTL       |                            ");
    fbPutString(" |CI   =           | |4.PFLAG        9.PFLAG       |                            ");
    fbPutString(" |SNTF =           | |5.PBC          9.PBC         |                            ");
    fbPutString(" |FBS  =           | +-----------------------------+                            ");
    fbPutString(" +-----------------+                                                            ");
}

void ahci_term_drawoverlay_command()
{
    
}

