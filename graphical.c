#include "graphical.h"

void drawTest(uint32_t videoMemBase)
{
    int x = 10;
    int y = 10;
    unsigned char color = 9;
    
    uint8_t* VGA = (uint8_t*)videoMemBase;//0xA0000;
    unsigned short offset;
    
    
    offset = 320*y + x;
    VGA[offset] = color;
}
