#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stddef.h>
#include <stdint.h>

#define FBADDR (uint16_t*)0xB8000
#define FBCOLS 80
#define FBROWS 25

enum VGACOLOR
{
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

// STATE VARIABLES
uint8_t fbColPos;
uint8_t fbRowPos;
uint16_t* fb;

uint8_t fbColorFront;
uint8_t fbColorBack;

// PRIVATE FUNCTIONS
uint16_t fbGetVGAEntry(char c, uint8_t forecolor, uint8_t backcolor);


// KERNEL FUNCTIONS
void fbInitialize();


// PUBLIC INTERFACE

void fbMoveCursor(uint8_t col, uint8_t row);
void fbPutChar(char c);
void fbSetColor(uint8_t forecolor, uint8_t backcolor);

// EXTENDED FUNCTIONS
void fbPutString(char* str);
void fbClear();
uint8_t fbGetColor(enum VGACOLOR front, enum VGACOLOR back);

#endif
