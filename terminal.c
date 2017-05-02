#include "terminal.h"

#include "framebuffer.h"
#include "memory.h"
#include "timer.h"
#include "common.h"

void term_init()
{
    RegisterKeyboardHook(&term_kb_hook);
    
    terminal* t = kmKernelAlloc(sizeof(terminal));
        
    term_setup(t);
    
    currentTerminal = t;
}

void term_setup(terminal* term)
{
    term->fbcontent = kmKernelAlloc(sizeof(char*) * (80*20));
    
    term->cursorX = 0;
    term->cursorY = 0;
    
    term->fbOriginX = 5;
    term->fbOriginY = 5;
    
    term->nCols = 10;
    term->nRows = 5;
    
    fbMoveCursor(term->fbOriginX, term->fbOriginY);
}

void term_update()
{
    lastTerminalUpdateTime = getmscount();
}

void term_draw(terminal* term)
{
    size_t baseX = term->fbOriginX;
    size_t baseY = term->fbOriginY;
    
    fbMoveCursor(baseX, baseY);
    
    size_t currentX = baseX;
    size_t currentY = baseY;
    
    size_t i = 0;
    
    while(currentY <= term->nRows)
    {
        while(currentX <= term->nCols)
        {
            fbPutChar(term->fbcontent[i]);
            
            i++;
            currentX++;
        }
        
        currentX = baseX;
        currentY++;
        
        fbMoveCursor(currentX, currentY);
    }
}

void term_kb_hook(keyevent_info* info)
{
    Debugger();
    
    uint32_t t = getmscount();
    
    if(info->key_state == KEYDOWN)
    {
        if(IsPrintableCharacter(info->key) == FALSE)
        {
            // Process ENTER and BACKSPACE and stuff
            if(info->key == ENTER)
            {
                currentTerminal->cursorY++;
            }
            
            return;
        }
        
        unsigned char c = GetAscii(info->key);
        
        if(t >= lastTerminalUpdateTime + KEYBOARD_REPEAT_TIME)
        {
            size_t pos = (currentTerminal->cursorY * 80) + currentTerminal->cursorX;
            
            currentTerminal->fbcontent[pos] = c;
            
            term_putChar(c);
        }
    }
    else
    {
        
    }
}

void term_putChar(unsigned char c)
{
    fbPutChar(c);
    
    currentTerminal->cursorX++;
    
    if(currentTerminal->cursorX >= currentTerminal->nCols)
    {
        currentTerminal->cursorX = 0;
        currentTerminal->cursorY++;
        
        if(currentTerminal->cursorY >= currentTerminal->nRows)
        {
            // Scroll down 1 line
            currentTerminal->cursorY = 0;
        }
    }
    
    fbMoveCursor(currentTerminal->fbOriginX + currentTerminal->cursorX, 
                    currentTerminal->fbOriginY + currentTerminal->cursorY);
}

void term_putString(unsigned char* str)
{
    int nextIndex = 0;
    
    while(str[nextIndex] != 0)
    {
        term_putChar(str[nextIndex]);
        
        
        
        
        nextIndex++;
    }
}
