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
    
    int cursorX;
    int cursorY;
    int fbOriginX;
    int fbOriginY;
    int nCols;
    int nRows;
    
} terminal;

terminal* currentTerminal;

// Timing values
uint32_t lastTerminalUpdateTime;

// KERNEL INTERFACE
void term_init();

void term_setup(terminal* term);

void term_update();

void term_draw(terminal* term);

// PUBLIC INTERFACE

void term_move_right();
void term_move_left();
void term_move_up();
void term_move_down();

void term_set_cursor(int col, int row);

void term_write(unsigned char c);
void term_erase();

void term_enter();

void term_showSplashScreen();

void term_putChar(unsigned char c);

// PRIVATE METHODS

void term_kb_hook(keyevent_info* info);

#endif
