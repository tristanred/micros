#ifndef AHCI_TERM_H
#define AHCI_TERM_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "keyboard.h"
#include "framebuffer.h"
#include "ahci_driver.h"

enum ahci_terminal_states
{
    MAIN_SCREEN,
    PORT_SCREEN,
    COMMAND_SCREEN
};

enum ahci_terminal_states current_state;
uint8_t view_port_nb;
uint8_t view_command_nb;

int commandLineIndex;
char commandLineEntry[64];
BOOL command_latch;

// int displayWidth;
// int displayHeight;
// char** display;

void init_ahci_term();

void ahci_term_update();
void ahci_term_drawdisplay();

void ahci_term_task();
void ahci_term_kbhook(keyevent_info* info);


void ahci_term_drawoverlay();
void ahci_term_drawoverlay_main();
void ahci_term_drawoverlay_port();
void ahci_term_drawoverlay_command();

#endif
