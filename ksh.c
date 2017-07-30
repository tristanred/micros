#include "ksh.h"

#include "string.h"
#include "array_utils.h"

BOOL ksh_take_fb_control()
{
    RegisterKeyboardHook(&ksh_kb_hook);

    promptText = "|>\0";
    promptLength = strlen(promptText);
    
    cursorColumn = 0;
    
    lines = (char**)malloc(sizeof(char*) * KSH_LINES_IN_MEMORY);
    lines[0] = (char*)malloc(sizeof(char) * KSH_MAX_LINE_LENGTH);
    strcpy(lines[0], "");
    
    for(int i = 1; i < KSH_LINES_IN_MEMORY; i++)
    {
        lines[i] = NULL;
    }
    
    fbClear();
    
    fbMoveCursor(0, ksh_get_line_number_from_bottom(0));
    
    ksh_render_line(0);
    
    return TRUE;
}

void ksh_fb_release()
{
    DeregisterKeyboardHook(&ksh_kb_hook);
}

void ksh_write(const char* characters)
{
    size_t cLen = strlen(characters);
    
    for(size_t i = 0; i < cLen; i++)
    {
        if(characters[i] == '\n')
        {
            ksh_push_lines();
        }
        else
        {
            char* line = ksh_get_current_type_line();
            line[cursorColumn] = characters[i];
            cursorColumn++;
        }
    }
}

void ksh_write_line(const char* line)
{
    ksh_push_lines();
    
    ksh_write(line);
}

void ksh_update()
{
    
}

void ksh_render_line(int nb)
{
    if(nb >= KSH_LINES_IN_MEMORY && lines[nb] == NULL)
        return;
    
    int screenRow = ksh_get_line_number_from_bottom(nb);
    
    if(screenRow < 0)
        return;
    
    fbMoveCursor(0, screenRow);
    
    if(ksh_is_current_type_line(nb))
    {
        // Draw prompt
        fbPutString(promptText);
    }
    
    fbPutString(lines[nb]);
}

void ksh_process_command(char* commandline)
{
    if(commandline[0] == 'p')
    {
        ksh_write_line("Test");
    }
    else if(strncmp(commandline, "dostuff", 7) == 0)
    {
        ksh_write_line("Doing stuff !");
    }
}

void ksh_type_character(char value)
{
    if(cursorColumn >= KSH_MAX_LINE_LENGTH - promptLength)
        return;
    
    char* currentLine = ksh_get_current_type_line();
    
    currentLine[cursorColumn] = value;
    
    cursorColumn++;
    
    currentLine[cursorColumn] = '\0';
    
    ksh_render_line(ksh_get_current_line_nb());
}

void ksh_erase_character()
{
    char* currentLine = ksh_get_current_type_line();
    
    cursorColumn--;

    currentLine[cursorColumn] = ' ';
    
    ksh_render_line(ksh_get_current_line_nb());
}

void ksh_enter_command()
{
    char* currentLine = ksh_get_current_type_line();
    
    // Process data
    ksh_process_command(currentLine);
    
    ksh_push_lines();
}

char* ksh_get_current_type_line()
{
    return lines[ksh_get_current_line_nb()];
}

BOOL ksh_is_current_type_line(int nb)
{
    return nb == 0;
}

int ksh_get_current_line_nb()
{
    return 0;
}

int ksh_get_line_number_from_bottom(int nb)
{
    // TODO : Verify FBROWS should be 25 or 24. 25 causes the need for -2
    return FBROWS - nb -2;
}

void ksh_push_lines()
{
    fbClear();
    fbMoveCursor(0, ksh_get_line_number_from_bottom(0));
    
    for(int i = KSH_LINES_IN_MEMORY; i > 0; i--)
    {
        if(lines[i - 1] != NULL)
        {
            lines[i] = lines[i - 1];
            
            ksh_render_line(i);
        }
    }
    
    char* result = (char*)malloc(sizeof(char) * KSH_MAX_LINE_LENGTH);
    lines[0] = result;
    
    array_set((uint8_t*)lines[0], 0, sizeof(char) * KSH_MAX_LINE_LENGTH);
    
    cursorColumn = 0;
    ksh_render_line(0);
    
    fbMoveCursor(0, ksh_get_line_number_from_bottom(0));
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
