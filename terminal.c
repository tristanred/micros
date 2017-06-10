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
    
    term->fbOriginX = 1;
    term->fbOriginY = 10;
    
    term->nCols = 78;
    term->nRows = 9;
    
    fbMoveCursor(term->fbOriginX, term->fbOriginY);
}

void term_update()
{
    lastTerminalUpdateTime = getmscount();
}

void term_draw(terminal* term)
{
    if(currentTerminal->isActive == FALSE)
        return;

    int baseX = term->fbOriginX;
    int baseY = term->fbOriginY;
    
    fbMoveCursor(baseX, baseY);
    
    int currentX = baseX;
    int currentY = baseY;
    
    int i = 0;
    
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
    if(currentTerminal->isActive == FALSE)
        return;
    
    uint32_t t = getmscount();
    
    if(info->key_state == KEYDOWN)
    {
        if(IsPrintableCharacter(info->key) == FALSE)
        {
            // Process ENTER and BACKSPACE and stuff
            if(info->key == ENTER)
            {
                term_enter();
            }
            else if(info->key == BACKSPACE)
            {
                term_erase();
            }
            
            return;
        }
        
        unsigned char c = GetAscii(info->key);
        
        if(t >= lastTerminalUpdateTime + KEYBOARD_REPEAT_TIME)
        {
            size_t pos = (currentTerminal->cursorY * 80) + currentTerminal->cursorX;
            
            currentTerminal->fbcontent[pos] = c;
            
            term_write(c);
        }
    }
    else
    {
        
    }
}

void term_activate()
{
    currentTerminal->isActive = TRUE;
}

void term_deactivate()
{
    currentTerminal->isActive = FALSE;
}

void term_move_right()
{
    if(currentTerminal->isActive == FALSE)
        return;

    currentTerminal->cursorX++;
    
    if(currentTerminal->cursorX >= currentTerminal->nCols)
    {
        currentTerminal->cursorX = 0;
        currentTerminal->cursorY++;
        
        if(currentTerminal->cursorY >= currentTerminal->nRows)
        {
            currentTerminal->cursorY = 0;
        }
    }
    
    term_set_cursor(currentTerminal->cursorX, currentTerminal->cursorY);
}

void term_move_left()
{
    if(currentTerminal->isActive == FALSE)
        return;

    currentTerminal->cursorX--;
    
    if(currentTerminal->cursorX < 0)
    {
        currentTerminal->cursorX = currentTerminal->nCols - 1;
        currentTerminal->cursorY--;
        
        if(currentTerminal->cursorY < 0)
        {
            currentTerminal->cursorY = 0;
        }
    }
    
    term_set_cursor(currentTerminal->cursorX, currentTerminal->cursorY);
}

void term_move_up()
{
    if(currentTerminal->isActive == FALSE)
        return;

    currentTerminal->cursorY--;
    
    if(currentTerminal->cursorY < 0)
    {
        currentTerminal->cursorY = 0;
    }
    
    term_set_cursor(currentTerminal->cursorX, currentTerminal->cursorY);
}

void term_move_down()
{
    if(currentTerminal->isActive == FALSE)
        return;

    currentTerminal->cursorY++;
    
    if(currentTerminal->cursorY >= currentTerminal->nRows)
    {
        currentTerminal->cursorY = currentTerminal->nRows - 1;
    }
    
    term_set_cursor(currentTerminal->cursorX, currentTerminal->cursorY);
}

void term_set_cursor(int col, int row)
{
    if(currentTerminal->isActive == FALSE)
        return;

    int tCol = col;
    if(col >= currentTerminal->nCols)
        tCol = currentTerminal->nCols;
    
    int tRow = row;
    if(row >= currentTerminal->nRows)
        tRow = currentTerminal->nRows;
        
    currentTerminal->cursorX = tCol;
    currentTerminal->cursorY = tRow;
    
    fbMoveCursor(currentTerminal->cursorX + currentTerminal->fbOriginX, 
                 currentTerminal->cursorY + currentTerminal->fbOriginY);
}

void term_write(unsigned char c)
{
    fbPutChar(c);
    
    term_move_right();
}

void term_erase()
{
    if(currentTerminal->cursorX == 0 && currentTerminal->cursorY == 0)
        return;
    
    term_move_left();
    
    fbPutChar(' ');
    
    term_set_cursor(currentTerminal->cursorX, currentTerminal->cursorY);    
}

void term_enter()
{
    term_set_cursor(0, currentTerminal->cursorY + 1);
}

void term_showSplashScreen()
{
    int currX = fbColPos;
    int currY = fbRowPos;
    
    fbMoveCursor(0, 0);
    
    fbPutString("                __  __ _____ _____ _____   ____   _____ \n");
    fbPutString("               |  \\/  |_   _/ ____|  __ \\ / __ \\ / ____|\n");
    fbPutString("               | \\  / | | || |    | |__) | |  | | (___  \n");
    fbPutString("               | |\\/| | | || |    |  _  /| |  | |\\___ \\ \n");
    fbPutString("               | |  | |_| || |____| | \\ \\| |__| |____) |\n");
    fbPutString("               |_|  |_|_____\\_____|_|  \\_\\ ____/|_____/ \n");
    
    fbMoveCursor(currX, currY);
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
