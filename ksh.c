#include "ksh.h"

#include "framebuffer.h"

BOOL ksh_take_fb_control()
{
    RegisterKeyboardHook(&ksh_kb_hook);

    promptText = "|>\0";

    cursorColumn = strlen(promptText);
    
    return TRUE;
}

void ksh_fb_release()
{
    DeregisterKeyboardHook(&ksh_kb_hook);
}

void ksh_render_line(int nb)
{
    
}

void ksh_type_character(char value)
{
    char* currentLine = ksh_get_current_type_line();
    
    currentLine[cursorColumn] = value;
    
    cursorColumn++;
}

void ksh_erase_character()
{
    cursorColumn--;
    
    char* currentLine = ksh_get_current_type_line();

    currentLine[cursorColumn] = '\0';
}

void ksh_enter_command()
{
    char* currentLine = ksh_get_current_type_line();
    
    // Process data
    //DoStuff();
    
    ksh_push_line();
}

char* ksh_get_current_type_line()
{
    if(linesCount < 1)
    {
        linesCount = 0;
        ksh_push_line();
    }
    
    return lines[linesCount - 1];
}

void ksh_push_line()
{
    char* newLine = malloc(sizeof(char) * KSH_MAX_LINE_LENGTH);
    lines[linesCount] = newLine;
    linesCount++;
    
    cursorColumn = 0;
}

void ksh_kb_hook(keyevent_info* info)
{
    if(info->key_state == KEYDOWN)
    {
        if(IsPrintableCharacter(info->key) == TRUE)
        {
            ksh_type_character(GetAscii(info->key));
        }
        else if(info->key == ENTER)
        {
            ksh_enter_command();
        }
        else if(info->key == LEFT_ARROW)
        {
            cursorColumn--;
        }
        else if(info->key == RIGHT_ARROW)
        {
            cursorColumn++;
        }
        else if(info->key == BACKSPACE)
        {
            ksh_erase_character();
        }
    }
}
