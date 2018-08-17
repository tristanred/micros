#include "ahci_terminal.h"

#include "memory.h"
#include "timer.h"
#include "idt.h"

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
    
    commandLineIndex = 0;
    command_latch = FALSE;
    memset(commandLineEntry, ' ', CMD_MAXLEN);
    commandLineEntry[CMD_MAXLEN] = '\0';
    cmdredraw = FALSE;
    
    previous_host = kmalloc(sizeof(struct ahci_host_regs));
    previous_ports = kmalloc(sizeof(struct ahci_port_regs));
}

void ahci_term_update()
{
    // Update stuff
    if(command_latch)
    {
        command_latch = FALSE;
        
        ahci_term_parse_cmd(commandLineEntry);
        
        memset(commandLineEntry, ' ', CMD_MAXLEN);
        commandLineIndex = 0;
    }
    
    // Draw stuff
    //ahci_term_drawoverlay();
    
    // Get the host regs (11 dwords)
    struct ahci_host_regs host;
    int res = driver_ahci_read_GHC_regs(&host);
    if(FAILED(res))
        goto error;
    
    char buf[256];

    if(current_state == MAIN_SCREEN)
    {
        // Check if the information changed before drawing on fb
        if(mcmp((uint8_t*)&host, (uint8_t*)previous_host, sizeof(struct ahci_host_regs) != 0))
        {
            memcpy(previous_host, &host, sizeof(struct ahci_host_regs));

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
        
        if(cmdredraw == TRUE)
        {
            fbMoveCursor(0, 20);
            fbPutString(commandLineEntry);
        }
        
    }
    else if(current_state == PORT_SCREEN)
    {
        Debugger();
        struct ahci_port_regs pr;
        res = driver_ahci_read_port_regs(view_port_nb, &pr);
        if(FAILED(res))
            goto error;
        
        if(mcmp((uint8_t*)&pr, (uint8_t*)previous_ports, sizeof(struct ahci_port_regs)) != 0)
        {
            memcpy(previous_ports, &pr, sizeof(struct ahci_port_regs));
            
            fbMoveCursor(9, 4);
            sprintf(buf, "%d", pr.command_list_base_addr_lower);
            fbPutString(buf);

            fbMoveCursor(9, 5);
            sprintf(buf, "%d", pr.command_list_base_addr_upper);
            fbPutString(buf);

            fbMoveCursor(9, 6);
            sprintf(buf, "%d", pr.fis_base_addr_lower);
            fbPutString(buf);

            fbMoveCursor(9, 7);
            sprintf(buf, "%d", pr.fis_base_addr_upper);
            fbPutString(buf);

            fbMoveCursor(9, 8);
            sprintf(buf, "%d", pr.interrupt_status);
            fbPutString(buf);

            fbMoveCursor(9, 9);
            sprintf(buf, "%d", pr.interrupt_enable);
            fbPutString(buf);

            fbMoveCursor(9, 10);
            sprintf(buf, "%d", pr.command_and_status);
            fbPutString(buf);

            fbMoveCursor(9, 11);
            sprintf(buf, "%d", pr.task_file_data);
            fbPutString(buf);

            fbMoveCursor(9, 12);
            sprintf(buf, "%d", pr.signature);
            fbPutString(buf);

            fbMoveCursor(9, 13);
            sprintf(buf, "%d", pr.serial_ata_status);
            fbPutString(buf);

            fbMoveCursor(9, 14);
            sprintf(buf, "%d", pr.serial_ata_control);
            fbPutString(buf);

            fbMoveCursor(9, 15);
            sprintf(buf, "%d", pr.serial_ata_error);
            fbPutString(buf);

            fbMoveCursor(9, 16);
            sprintf(buf, "%d", pr.serial_ata_active);
            fbPutString(buf);

            fbMoveCursor(9, 17);
            sprintf(buf, "%d", pr.serial_command_issue);
            fbPutString(buf);

            fbMoveCursor(9, 18);
            sprintf(buf, "%d", pr.serial_ata_notification);
            fbPutString(buf);

            fbMoveCursor(9, 19);
            sprintf(buf, "%d", pr.fis_based_switching_control);
            fbPutString(buf);

            // Get the list of active commands
        }
        
        if(cmdredraw == TRUE)
        {
            fbMoveCursor(21, 20);
            fbPutString(commandLineEntry);
        }
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
    ahci_term_drawoverlay();
    
    while(TRUE)
    {
        ahci_term_update();
        sleep(1);
    }
}

void ahci_term_kbhook(keyevent_info* info)
{
    if(info->key_state == KEYDOWN)
    {
        cmdredraw = TRUE;
        
        if(IsPrintableCharacter(info->key) == TRUE)
        {
            commandLineEntry[commandLineIndex] = GetAscii(info->key);
            commandLineIndex++;
        }
        else if(info->key == BACKSPACE)
        {
            commandLineIndex--;
            commandLineEntry[commandLineIndex] = ' ';
        }
        else if(info->key == ESCAPE)
        {
            memset(commandLineEntry, ' ', CMD_MAXLEN);
        }
        else if(info->key == ENTER)
        {
            command_latch = TRUE;
        }
    }
}

void ahci_term_parse_cmd(const char* cmdline)
{
    switch(current_state)
    {
        case MAIN_SCREEN:
        {
            if(cmdline[0] == 'p')
            {
                char portNumberStr[3];
                strncpy(portNumberStr, cmdline+1, 2);
                
                int portNumber = (uint8_t)atoi(portNumberStr);
                
                view_port_nb = portNumber;
                current_state = PORT_SCREEN;
                ahci_term_drawoverlay();
                
                // To make sure it changes and causes a redraw
                memset(previous_ports, 0, sizeof(struct ahci_port_regs));
            }
            
            break;
        }
        case PORT_SCREEN:
        {
            if(cmdline[0] == 'h')
            {
                current_state = MAIN_SCREEN;
                memset(previous_host, 0, sizeof(struct ahci_host_regs));
                ahci_term_drawoverlay();
            }
            
            break;
        }
        case COMMAND_SCREEN:
        {
            break;
        }
    }
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

