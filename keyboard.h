#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stddef.h>
#include <stdint.h>

#include "idt.h"


struct keyboard_state
{
    uint8_t currentScancode;
    uint8_t currentKeycode;
} current_keyboard_state;

void keyboard_interrupt_handler(registers_t regs);

#endif
