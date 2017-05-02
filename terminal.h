#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>
#include <stddef.h>

#include "keyboard.h"

#define MAX_TERMINALS 4

#define KEYBOARD_REPEAT_TIME 50

typedef struct
{
    unsigned char* fbcontent;
    
    size_t cursorX;
    size_t cursorY;
    
    size_t fbOriginX;
    size_t fbOriginY;
    size_t nCols;
    size_t nRows;
    
} terminal;

terminal* terminals[MAX_TERMINALS];
size_t currentTerminalIndex;
terminal* currentTerminal;

// Timing values
uint32_t lastTerminalUpdateTime;

// KERNEL INTERFACE
void term_init();

void term_setup(terminal* term);

void term_update();

void term_switch(size_t termNb);

void term_draw(terminal* term);

// PUBLIC INTERFACE

void term_showSplashScreen();

void term_putChar(unsigned char c);

void term_putString(unsigned char* str);

// PRIVATE METHODS

void term_kb_hook(keyevent_info* info);

#endif
