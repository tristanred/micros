#include "framebuffer.h"
#include "string.h"
#include "io_func.h"
#include "runflags.h"

/*
 * Framebuffer data format
 *
 * Bit:     | 15 14 13 12 11 10 9 8 | 7 6 5 4 | 3 2 1 0 |
 * Content: | ASCII                 | FG      | BG      |
 */


void fbInitialize()
{
    fb = FBADDR;
    fbColPos = 0;
    fbRowPos = 0;
    fbColorFront = VGA_COLOR_WHITE;
    fbColorBack = VGA_COLOR_BLACK;
    
    for(uint32_t i = 0; i < FBROWS; i++)
    {
        for(uint32_t j = 0; j < FBCOLS; j++)
        {
            int offset = (i * FBCOLS) + j;
            
            // BUG 24/03/2017 : Setting 2nd and 3rd parameter of this call
            // will add an extra parameter to the kernel_main function and
            // cause an early exit because a paremeter is not null. Weird.
            fb[offset] = fbGetVGAEntry(' ', fbColorFront, fbColorBack);
        }
    }
    
#if ENV_TMUX == 1
    fbColPos = 1;
#endif
}

void fbMoveCursor(uint8_t col, uint8_t row)
{
    fbColPos = col;
    fbRowPos = row;
    
    uint16_t pos = col + (row * FBCOLS);
    
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    outb(FB_DATA_PORT,    ((pos >> 8) & 0x00FF));
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    outb(FB_DATA_PORT,    pos & 0x00FF);
}

void fbPutChar(char c)
{
    int offset = (fbRowPos * FBCOLS) + fbColPos;
    
    fb[offset] = fbGetVGAEntry(c, fbColorFront, fbColorBack);
    
    fbColPos++;
    
    if(fbColPos >= FBCOLS)
    {
        fbColPos = 0;
        fbRowPos++;
        
        if(fbRowPos >= FBROWS)
        {
            fbRowPos = 0;
        }
    }
}

uint16_t fbGetVGAEntry(char c, uint8_t forecolor, uint8_t backcolor)
{
    return (uint16_t)c | ((uint16_t)backcolor << 4 | (uint16_t)forecolor) << 8;
}

void fbSetColor(uint8_t forecolor, uint8_t backcolor)
{
    fbColorFront = forecolor;
    fbColorBack = backcolor;
}

void fbPutString(char* str)
{
    size_t stringLengthSafe = strlen_s(str, FBROWS * FBCOLS);
    
    for(uint32_t i = 0; i < stringLengthSafe; i++)
    {
        fbPutChar(str[i]);
        
        if(str[i] == '\n')
        {
            fbMoveCursor(0, fbRowPos + 1);
        }
    }
}

void fbClear()
{
    fbMoveCursor(0, 0);
    
    for(uint32_t i = 0; i < FBROWS * FBCOLS; i++)
    {
        fbPutChar(' ');
    }
    
#if ENV_TMUX == 1
    fbMoveCursor(0, 1);
#endif
}

uint8_t fbGetColor(enum VGACOLOR front, enum VGACOLOR back)
{
    return (uint8_t)back << 8 | (uint8_t)front << 4;
}
