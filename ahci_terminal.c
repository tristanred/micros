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

    for(int i = 0; i < MAIN_SHOWPORTS_NB; i++)
    {
        main_previous_ports[i] = kmalloc(sizeof(struct ahci_port_regs));
    }
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
        if(ahci_term_check_main_redraw(&host) == TRUE)
        {
            ahci_term_draw_values_main(&host);
        }

        if(cmdredraw == TRUE)
        {
            fbMoveCursor(9, 16);
            fbPutString(commandLineEntry);
        }
    }
    else if(current_state == MAIN_SCREEN_CAP)
    {
        if(ahci_term_check_main_cap_redraw(host.host_capabilities) == TRUE)
        {
            ahci_term_draw_values_main_host_cap(host.host_capabilities);
        }

        if(cmdredraw == TRUE)
        {
            fbMoveCursor(47, 17);
            fbPutString(commandLineEntry);
        }
    }
    else if(current_state == MAIN_SCREEN_GHC)
    {
        if(ahci_term_check_main_ghc_redraw(host.global_host_control) == TRUE)
        {
            ahci_term_draw_values_main_host_ghc(host.global_host_control);
        }

        if(cmdredraw == TRUE)
        {
            fbMoveCursor(8, 11);
            fbPutString(commandLineEntry);
        }
    }
    else if(current_state == MAIN_SCREEN_ISR)
    {
        if(ahci_term_check_main_isr_redraw(host.interrupt_status) == TRUE)
        {
            ahci_term_draw_values_main_host_isr(host.interrupt_status);
        }

        if(cmdredraw == TRUE)
        {
            fbMoveCursor(42, 16);
            fbPutString(commandLineEntry);
        }
    }
    else if(current_state == PORT_SCREEN)
    {
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
            else if(cmdline[0] == 'r')
            {
                Debugger();
                int res = driver_ahci_reset_controller();
                if(SUCCESS(res))
                {
                    kWriteLog("HBA Reset success !");
                }
                else
                {
                    kWriteLog("HBA Reset failed : %d", res);
                }
            }
            else if(cmdline[0] == '1')
            {
                Debugger();

                current_state = MAIN_SCREEN_CAP;
                ahci_term_drawoverlay();
            }
            else if(cmdline[0] == '2')
            {
                Debugger();

                current_state = MAIN_SCREEN_GHC;
                ahci_term_drawoverlay();
            }
            else if(cmdline[0] == '3')
            {
                current_state = MAIN_SCREEN_ISR;
                ahci_term_drawoverlay();

                // Temp hack to force refresh
                previous_isr_values = (uint32_t)-1;
            }

            break;
        }
        case MAIN_SCREEN_CAP:
        case MAIN_SCREEN_GHC:
        case MAIN_SCREEN_ISR:
        {
            if(cmdline[0] == 'h')
            {
                current_state = MAIN_SCREEN;
                memset(previous_host, 0, sizeof(struct ahci_host_regs));
                ahci_term_drawoverlay();
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
            else if(cmdline[0] == 'r')
            {
                int res = driver_ahci_reset_port(view_port_nb);
                if(SUCCESS(res))
                {
                    kWriteLog("Port %d Reset success !", view_port_nb);
                }
                else
                {
                    kWriteLog("Port %d Reset failed : %d", view_port_nb, res);
                }
            }
            else if(cmdline[0] == 'w')
            {
                Debugger();
                uint8_t mybuf[4096];
                memset(mybuf, 1, 4096);
                int res = driver_ahci_read_data(view_port_nb, 0, 0, 4096, mybuf);

                struct ata_identify_device ident;
                memset(&ident, 0, sizeof(struct ata_identify_device));
                res = driver_ahci_identify(view_port_nb, &ident);

                if(res == FALSE)
                    kWriteLog("Failed to call IDENTIFY");
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
        case MAIN_SCREEN_CAP:
        {
            ahci_term_drawoverlay_main_host_cap();
            break;
        }
        case MAIN_SCREEN_GHC:
        {
            ahci_term_drawoverlay_main_host_ghc();
            break;
        }
        case MAIN_SCREEN_ISR:
        {
            ahci_term_drawoverlay_main_host_isr();
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
    fbPutString("   CMD =                                                                        ");
    fbPutString("   (1) Host Cap     (5) Version       (9) EM  CTL      (r) Reset HBA            ");
    fbPutString("   (2) GBL Host Ctl (6) CCC  CTL     (10) Host Cap Ext                          ");
    fbPutString("   (3) Interrupt St (7) CCC Ports    (11) Bios Handoff                          ");
    fbPutString("   (4) Ports Impl   (8) EM  Location (p0-9) Goto Port                           ");
}

BOOL ahci_term_check_main_redraw(struct ahci_host_regs* regs)
{
    BOOL should_redraw = FALSE;

    if(mcmp((uint8_t*)regs, (uint8_t*)previous_host, sizeof(struct ahci_host_regs)) != 0)
    {
        memcpy(previous_host, regs, sizeof(struct ahci_host_regs));

        should_redraw = TRUE;
    }

    int res = 0;
    struct ahci_port_regs current_port_regs[MAIN_SHOWPORTS_NB];
    for(int i = 0; i < MAIN_SHOWPORTS_NB; i++)
    {
        res = driver_ahci_read_port_regs(i, &current_port_regs[i]);

        if(FAILED(res))
            return FALSE;

        if(mcmp((uint8_t*)&current_port_regs[i], (uint8_t*)main_previous_ports[i], sizeof(struct ahci_port_regs)) != 0)
        {
            memcpy(main_previous_ports[i], &current_port_regs[i], sizeof(struct ahci_port_regs));

            should_redraw = TRUE;
        }
    }

    return should_redraw;
}

void ahci_term_draw_values_main(struct ahci_host_regs* regs)
{
    char buf[256];

    fbMoveCursor(17, 4);
    sprintf(buf, "%d", regs->host_capabilities);
    fbPutString(buf);

    fbMoveCursor(17, 5);
    sprintf(buf, "%d", regs->global_host_control);
    fbPutString(buf);

    fbMoveCursor(17, 6);
    sprintf(buf, "%d", regs->interrupt_status);
    fbPutString(buf);

    fbMoveCursor(17, 7);
    sprintf(buf, "%d", regs->ports_implemented);
    fbPutString(buf);

    fbMoveCursor(17, 8);
    sprintf(buf, "%d", regs->version);
    fbPutString(buf);

    fbMoveCursor(17, 9);
    sprintf(buf, "%d", regs->command_completion_coalescing_control);
    fbPutString(buf);

    fbMoveCursor(17, 10);
    sprintf(buf, "%d", regs->command_completion_coalescing_ports);
    fbPutString(buf);

    fbMoveCursor(17, 11);
    sprintf(buf, "%d", regs->enclosure_management_location);
    fbPutString(buf);

    fbMoveCursor(17, 12);
    sprintf(buf, "%d", regs->enclosure_management_control);
    fbPutString(buf);

    fbMoveCursor(17, 13);
    sprintf(buf, "%d", regs->host_capabilities_extended);
    fbPutString(buf);

    fbMoveCursor(17, 14);
    sprintf(buf, "%d", regs->bios_handoff_control_status);
    fbPutString(buf);

    uint8_t ports[32] = { 0 };
    uint8_t portNb = 0;
    int res = driver_ahci_get_disk_ports(ports, &portNb);
    if(FAILED(res))
        ASSERT(FALSE, "Oh shit");

    for(uint8_t i = 0; i < portNb; i++)
    {
        struct ahci_port_regs portregs;
        res = driver_ahci_read_port_regs(ports[i], &portregs);
        if(FAILED(res))
            ASSERT(FALSE, "Oh shit");

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

void ahci_term_drawoverlay_main_host_cap()
{
    fbMoveCursor(0, 0);
    fbPutString(" +-----------------------+                                                      ");
    fbPutString(" | Host Cap =            |                                                      ");
    fbPutString(" +-----------------------+                                                      ");
    fbPutString("  31   (S64A) Sup 64bit Addr        =       06   (EMS) Encl Manage Sup  =       ");
    fbPutString("  30   (SNCQ) Sup Native Cmd Que    =       05   (SXS) Sup Extern SATA  =       ");
    fbPutString("  29  (SSNTF) Sup SNotif Reg        =      00:04 (NP)  NB Ports         =       ");
    fbPutString("  28   (SMPS) Sup Mech Pres Swch    =                                           ");
    fbPutString("  27   (SSS)  Sup Stagger Spinup    =                                           ");
    fbPutString("  26   (SALP) Sup Aggr Link PW Man  =                                           ");
    fbPutString("  25   (SAL)  Sup Activity LED      =                                           ");
    fbPutString("  24   (SCLO) Sup Cmd List Override =                                           ");
    fbPutString(" 23:20 (ISS)  if Speed Support      =                                           ");
    fbPutString("  18   (SAM)  Sup AHCI only         =                                           ");
    fbPutString("  17   (SPM)  Sup Port Multiplier   =                                           ");
    fbPutString("  16   (FBSS) FIS switch Supp       =                                           ");
    fbPutString("  15   (PMD)  PIO Mult DRQ blok     =                                           ");
    fbPutString("  14   (SSC)  Slumber State Capable =    CMD =                                  ");
    fbPutString("  13   (PSC)  Partial State Capable =                                           ");
    fbPutString(" 12:08 (NCS)  Nb Cmd Slot           =                                           ");
    fbPutString("  07   (CCCS) Cmd Cmpl Coales       =                                           ");
}

BOOL ahci_term_check_main_cap_redraw(uint32_t reg)
{
    if(reg != previous_cap_reg_value)
    {
        previous_cap_reg_value = reg;

        return TRUE;
    }

    return FALSE;
}

void ahci_term_draw_values_main_host_cap(uint32_t reg)
{
    char buf[256];

    fbMoveCursor(14, 2);
    sprintf(buf, "%d", reg);
    fbPutString(buf);

    fbMoveCursor(38, 4);
    sprintf(buf, "%b", AHCI_CAP_S64A(reg));
    fbPutString(buf);

    fbMoveCursor(38, 5);
    sprintf(buf, "%b", AHCI_CAP_SNCQ(reg));
    fbPutString(buf);

    fbMoveCursor(38, 6);
    sprintf(buf, "%b", AHCI_CAP_SSNTF(reg));
    fbPutString(buf);

    fbMoveCursor(38, 7);
    sprintf(buf, "%b", AHCI_CAP_SMPS(reg));
    fbPutString(buf);

    fbMoveCursor(38, 8);
    sprintf(buf, "%b", AHCI_CAP_SSS(reg));
    fbPutString(buf);

    fbMoveCursor(38, 9);
    sprintf(buf, "%b", AHCI_CAP_SALP(reg));
    fbPutString(buf);

    fbMoveCursor(38, 10);
    sprintf(buf, "%b", AHCI_CAP_SAL(reg));
    fbPutString(buf);

    fbMoveCursor(38, 11);
    sprintf(buf, "%b", AHCI_CAP_SCLO(reg));
    fbPutString(buf);

    fbMoveCursor(38, 12);
    sprintf(buf, "%d", AHCI_CAP_ISS(reg));
    fbPutString(buf);

    fbMoveCursor(38, 13);
    sprintf(buf, "%b", AHCI_CAP_SAM(reg));
    fbPutString(buf);

    fbMoveCursor(38, 14);
    sprintf(buf, "%b", AHCI_CAP_SPM(reg));
    fbPutString(buf);

    fbMoveCursor(38, 15);
    sprintf(buf, "%b", AHCI_CAP_FBSS(reg));
    fbPutString(buf);

    fbMoveCursor(38, 16);
    sprintf(buf, "%b", AHCI_CAP_PMD(reg));
    fbPutString(buf);

    fbMoveCursor(38, 17);
    sprintf(buf, "%b", AHCI_CAP_SSC(reg));
    fbPutString(buf);

    fbMoveCursor(38, 18);
    sprintf(buf, "%b", AHCI_CAP_PSC(reg));
    fbPutString(buf);

    fbMoveCursor(38, 19);
    sprintf(buf, "%d", AHCI_CAP_NCS(reg) >> 8);
    fbPutString(buf);

    fbMoveCursor(38, 20);
    sprintf(buf, "%b", AHCI_CAP_CCCS(reg));
    fbPutString(buf);

    fbMoveCursor(74, 4);
    sprintf(buf, "%b", AHCI_CAP_EMS(reg));
    fbPutString(buf);

    fbMoveCursor(74, 5);
    sprintf(buf, "%b", AHCI_CAP_SXS(reg));
    fbPutString(buf);

    fbMoveCursor(74, 6);
    sprintf(buf, "%d", AHCI_CAP_NP(reg));
    fbPutString(buf);
}

void ahci_term_drawoverlay_main_host_ghc()
{
    fbMoveCursor(0, 0);
    fbPutString(" +-----------------------+                                                      ");
    fbPutString(" | Host Ctl =            |                                                      ");
    fbPutString(" +-----------------------+                                                      ");
    fbPutString("  31   (AE)   AHCI Enable           =                                           ");
    fbPutString("  02   (MRSM) MSI Revert Singl Msg  =                                           ");
    fbPutString("  01   (IE)   Interrupt Enable      =                                           ");
    fbPutString("  28   (HR)   HBA Reset             =                                           ");
    fbPutString("                                                                                ");
    fbPutString("                                                                                ");
    fbPutString("                                                                                ");
    fbPutString("  CMD =                                                                         ");
    fbPutString("                                                                                ");
    fbPutString("                                                                                ");
    fbPutString("                                                                                ");
    fbPutString("                                                                                ");
    fbPutString("                                                                                ");
    fbPutString("                                                                                ");
    fbPutString("                                                                                ");
    fbPutString("                                                                                ");
    fbPutString("                                                                                ");
}

BOOL ahci_term_check_main_ghc_redraw(uint32_t reg)
{
    if(reg != previous_ghc_reg_value)
    {
        previous_cap_reg_value = reg;

        return TRUE;
    }

    return FALSE;
}

void ahci_term_draw_values_main_host_ghc(uint32_t reg)
{
    char buf[256];

    fbMoveCursor(14, 2);
    sprintf(buf, "%d", reg);
    fbPutString(buf);

    fbMoveCursor(38, 4);
    sprintf(buf, "%b", AHCI_GHC_AE(reg));
    fbPutString(buf);

    fbMoveCursor(38, 5);
    sprintf(buf, "%b", AHCI_GHC_MRSM(reg));
    fbPutString(buf);

    fbMoveCursor(38, 6);
    sprintf(buf, "%b", AHCI_GHC_IE(reg));
    fbPutString(buf);

    fbMoveCursor(38, 7);
    sprintf(buf, "%b", AHCI_GHC_HR(reg));
    fbPutString(buf);
}

void ahci_term_drawoverlay_main_host_isr()
{
    fbMoveCursor(0, 0);
    fbPutString(" +-----------------------+                                                      ");
    fbPutString(" | IR Pend  = 4294967295 |                                                      ");
    fbPutString(" +-----------------------+                                                      ");
    fbPutString(" 0 =        9  =        18 =        27 =                                        ");
    fbPutString("                                                                                ");
    fbPutString(" 1 =        10 =        19 =        28 =                                        ");
    fbPutString("                                                                                ");
    fbPutString(" 2 =        11 =        20 =        29 =                                        ");
    fbPutString("                                                                                ");
    fbPutString(" 3 =        12 =        21 =        30 =                                        ");
    fbPutString("                                                                                ");
    fbPutString(" 4 =        13 =        22 =        31 =                                        ");
    fbPutString("                                                                                ");
    fbPutString(" 5 =        14 =        23 =                                                    ");
    fbPutString("                                                                                ");
    fbPutString(" 6 =        15 =        24 =        CMD =                                       ");
    fbPutString("                                                                                ");
    fbPutString(" 7 =        16 =        25 =                                                    ");
    fbPutString("                                                                                ");
    fbPutString(" 8 =        17 =        26 =                                                    ");
}

BOOL ahci_term_check_main_isr_redraw(uint32_t reg)
{
    if(reg != previous_isr_values)
    {
        previous_isr_values = reg;

        return TRUE;
    }

    return FALSE;
}

void ahci_term_draw_values_main_host_isr(uint32_t reg)
{
    Debugger();

    char buf[256];

    fbMoveCursor(14, 2);
    sprintf(buf, "%d", reg);
    fbPutString(buf);

    // Port 0
    fbMoveCursor(5, 4);
    sprintf(buf, "%b", (reg & 1<<0));
    fbPutString(buf);

    // Port 1
    fbMoveCursor(5, 6);
    sprintf(buf, "%b", (reg & 1<<1));
    fbPutString(buf);

    // Port 2
    fbMoveCursor(5, 8);
    sprintf(buf, "%b", (reg & 1<<2));
    fbPutString(buf);

    // Port 3
    fbMoveCursor(5, 10);
    sprintf(buf, "%b", (reg & 1<<3));
    fbPutString(buf);

    // Port 4
    fbMoveCursor(5, 12);
    sprintf(buf, "%b", (reg & 1<<4));
    fbPutString(buf);

    // Port 5
    fbMoveCursor(5, 14);
    sprintf(buf, "%b", (reg & 1<<5));
    fbPutString(buf);

    // Port 6
    fbMoveCursor(5, 16);
    sprintf(buf, "%b", (reg & 1<<6));
    fbPutString(buf);

    // Port 7
    fbMoveCursor(5, 18);
    sprintf(buf, "%b", (reg & 1<<7));
    fbPutString(buf);

    // Port 8
    fbMoveCursor(5, 20);
    sprintf(buf, "%b", (reg & 1<<8));
    fbPutString(buf);

    // Port 9
    fbMoveCursor(17, 4);
    sprintf(buf, "%b", (reg & 1<<9));
    fbPutString(buf);

    // Port 10
    fbMoveCursor(17, 6);
    sprintf(buf, "%b", (reg & 1<<10));
    fbPutString(buf);

    // Port 11
    fbMoveCursor(17, 8);
    sprintf(buf, "%b", (reg & 1<<11));
    fbPutString(buf);

    // Port 12
    fbMoveCursor(17, 10);
    sprintf(buf, "%b", (reg & 1<<12));
    fbPutString(buf);

    // Port 13
    fbMoveCursor(17, 12);
    sprintf(buf, "%b", (reg & 1<<13));
    fbPutString(buf);

    // Port 14
    fbMoveCursor(17, 14);
    sprintf(buf, "%b", (reg & 1<<14));
    fbPutString(buf);

    // Port 15
    fbMoveCursor(17, 16);
    sprintf(buf, "%b", (reg & 1<<15));
    fbPutString(buf);

    // Port 16
    fbMoveCursor(17, 18);
    sprintf(buf, "%b", (reg & 1<<16));
    fbPutString(buf);

    // Port 17
    fbMoveCursor(17, 20);
    sprintf(buf, "%b", (reg & 1<<17));
    fbPutString(buf);

    // Port 18
    fbMoveCursor(29, 4);
    sprintf(buf, "%b", (reg & 1<<18));
    fbPutString(buf);

    // Port 19
    fbMoveCursor(29, 6);
    sprintf(buf, "%b", (reg & 1<<19));
    fbPutString(buf);

    // Port 20
    fbMoveCursor(29, 8);
    sprintf(buf, "%b", (reg & 1<<20));
    fbPutString(buf);

    // Port 21
    fbMoveCursor(29, 10);
    sprintf(buf, "%b", (reg & 1<<21));
    fbPutString(buf);

    // Port 22
    fbMoveCursor(29, 12);
    sprintf(buf, "%b", (reg & 1<<22));
    fbPutString(buf);

    // Port 23
    fbMoveCursor(29, 14);
    sprintf(buf, "%b", (reg & 1<<23));
    fbPutString(buf);

    // Port 24
    fbMoveCursor(29, 16);
    sprintf(buf, "%b", (reg & 1<<24));
    fbPutString(buf);

    // Port 25
    fbMoveCursor(29, 18);
    sprintf(buf, "%b", (reg & 1<<25));
    fbPutString(buf);

    // Port 26
    fbMoveCursor(29, 20);
    sprintf(buf, "%b", (reg & 1<<26));
    fbPutString(buf);

    // Port 27
    fbMoveCursor(41, 4);
    sprintf(buf, "%b", (reg & 1<<27));
    fbPutString(buf);

    // Port 28
    fbMoveCursor(41, 6);
    sprintf(buf, "%b", (reg & 1<<28));
    fbPutString(buf);

    // Port 29
    fbMoveCursor(41, 8);
    sprintf(buf, "%b", (reg & 1<<29));
    fbPutString(buf);

    // Port 30
    fbMoveCursor(41, 10);
    sprintf(buf, "%b", (reg & 1<<30));
    fbPutString(buf);

    // Port 31
    fbMoveCursor(41, 12);
    sprintf(buf, "%b", (reg & 1<<31));
    fbPutString(buf);
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

void ahci_term_update_main()
{

}

void ahci_term_update_port()
{

}

BOOL ahci_term_check_port_redraw()
{
    return FALSE;
}

void ahci_term_log_main(struct ahci_host_regs* regs)
{
    kWriteLog("AHCI HOST REGS :");
    kWriteLog("host_capabilities = %d", regs->host_capabilities);
    kWriteLog("global_host_control = %d", regs->global_host_control);
    kWriteLog("interrupt_status = %d", regs->interrupt_status);
    kWriteLog("ports_implemented = %d", regs->ports_implemented);
    kWriteLog("version = %d", regs->version);
    kWriteLog("command_completion_coalescing_control = %d", regs->command_completion_coalescing_control);
    kWriteLog("command_completion_coalescing_ports = %d", regs->command_completion_coalescing_ports);
    kWriteLog("enclosure_management_location = %d", regs->enclosure_management_location);
    kWriteLog("enclosure_management_control = %d", regs->enclosure_management_control);
    kWriteLog("host_capabilities_extended = %d", regs->host_capabilities_extended);
    kWriteLog("bios_handoff_control_status = %d", regs->bios_handoff_control_status);
    kWriteLog("\n\n");
}

void ahci_term_log_port(struct ahci_port_regs* regs)
{
    kWriteLog("AHCI PORT REGS :");
    kWriteLog("command_list_base_addr_lower = %d", regs->command_list_base_addr_lower);
    kWriteLog("command_list_base_addr_upper = %d", regs->command_list_base_addr_upper);
    kWriteLog("fis_base_addr_lower = %d", regs->fis_base_addr_lower);
    kWriteLog("fis_base_addr_upper = %d", regs->fis_base_addr_upper);
    kWriteLog("interrupt_status = %d", regs->interrupt_status);
    kWriteLog("interrupt_enable = %d", regs->interrupt_enable);
    kWriteLog("command_and_status = %d", regs->command_and_status);
    kWriteLog("reserved1 = %d", regs->reserved1);
    kWriteLog("task_file_data = %d", regs->task_file_data);
    kWriteLog("signature = %d", regs->signature);
    kWriteLog("serial_ata_status = %d", regs->serial_ata_status);
    kWriteLog("serial_ata_control = %d", regs->serial_ata_control);
    kWriteLog("serial_ata_error = %d", regs->serial_ata_error);
    kWriteLog("serial_ata_active = %d", regs->serial_ata_active);
    kWriteLog("serial_command_issue = %d", regs->serial_command_issue);
    kWriteLog("serial_ata_notification = %d", regs->serial_ata_notification);
    kWriteLog("fis_based_switching_control = %d", regs->fis_based_switching_control);
    kWriteLog("device_sleep = %d", regs->device_sleep);
    kWriteLog("reserved2 = %d", regs->reserved2);
    kWriteLog("vendor_specific = %d", regs->vendor_specific);

    kWriteLog("\n\n");
}
