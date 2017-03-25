#include "framebuffer.h"
#include "string.h"

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
            int offset = (i * FBROWS) + j;
            
            fb[offset] = 0;
        }
    }
}

void fbMoveCursor(uint8_t col, uint8_t row)
{
    fbColPos = col;
    fbRowPos = row;
}

void fbPutChar(char c)
{
    int offset = (fbRowPos * FBROWS) + fbColPos;
    
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
    }
}

void fbClear()
{
    fbMoveCursor(0, 0);
    
    for(uint32_t i = 0; i < FBROWS * FBCOLS; i++)
    {
        fbPutChar(' ');
    }
}

uint8_t fbGetColor(enum VGACOLOR front, enum VGACOLOR back)
{
    return (uint8_t)back << 8 | (uint8_t)front << 4;
}
