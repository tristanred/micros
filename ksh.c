#include "ksh.h"

#include "string.h"
#include "array_utils.h"

#include "ezfs.h"
#include "ata_driver.h"

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
    ksh_push_lines();
    
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
    
    char* line = ksh_get_current_type_line();
    
    line[cursorColumn] = '\0';
}

void ksh_write_line(const char* line)
{
    ksh_write(line);
    ksh_write("");
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
    if(strcmp(commandline, "") == 0)
    {
        return;
    }
    
    size_t nb = 0;
    char** parts = strspl(commandline, " ", &nb);
    
    BOOL success = FALSE;
    
    if(nb > 0)
    {
        if(strcmp(parts[0], "echo") == 0)
        {
            if(nb > 1)
            {
                for(size_t i = 1; i < nb; i++)
                {
                    ksh_write(parts[i]);
                }
                
                success = TRUE;
            }
        }
        else if(strcmp(parts[0], "dwp") == 0)
        {
            // dwp 512 255 1500
            if(nb > 3)
            {
                char* bytesAmount = parts[1];
                char* pattern = parts[2];
                char* address = parts[3];
                
                uint32_t amountAsDWord = (uint32_t)s_to_d(bytesAmount);
                uint8_t patternAsByte = (uint8_t)s_to_d(pattern);
                int addressAsQWord = (int)s_to_d(address);
                
                uint8_t* writeData = (uint8_t*)malloc(amountAsDWord * sizeof(uint8_t));
                
                array_set_pattern(writeData, &patternAsByte, amountAsDWord, 1);
                
                write_data(writeData, amountAsDWord, addressAsQWord);
                
                success = TRUE;
            }
        }
        else if(strcmp(parts[0], "fread") == 0)
        {
            if(nb > 1)
            {
                char* fileName = parts[1];
                
                file_h file = ezfs_find_file(fileName);
                if(file != FILE_NOT_FOUND)
                {
                    uint8_t* buf = NULL;
                    size_t bytesRead = ezfs_read_file(file, &buf);
                    
                    char* bufToStr = (char*)malloc(sizeof(char) * bytesRead + 1);
                    
                    strncpy(bufToStr, (char*)buf, bytesRead);
                    
                    ksh_write_line(bufToStr);
                    
                    free(buf);
                    free(bufToStr);
                }
                else
                {
                    ksh_write_line("File not found.");
                }
                
                success = TRUE;
            }
        }
        else if(strcmp(parts[0], "fcreate") == 0)
        {
            if(nb > 2)
            {
                char* fileName = parts[1];
                char* fileData = parts[2];
                
                size_t dataLen =  strlen(fileData);
                
                file_h file = ezfs_create_file(ROOT_DIR, fileName, FS_READ_WRITE, FS_FLAGS_NONE);
                
                size_t res = ezfs_write_file(file, (uint8_t*)fileData, dataLen);
                
                ASSERT(res == dataLen, "WRONG SIZE WRITE");
                
                success = TRUE;
            }
        }
        else if(strcmp(parts[0], "fwrite") == 0)
        {
            if(nb > 2)
            {
                char* fileName = parts[1];
                char* fileData = parts[2];
                
                size_t dataLen = strlen(fileData);
                
                file_h file = ezfs_find_file(fileName);
                
                if(file != FILE_NOT_FOUND)
                {
                    size_t res = ezfs_write_file(file, (uint8_t*)fileData, dataLen);
                    
                    ASSERT(res == dataLen, "WRONG SIZE WRITE");
                }
                else
                {
                    ksh_write("File not found.");
                }
                
                success = TRUE;
            }
        }
        else if(strcmp(parts[0], "fappend") == 0)
        {
            if(nb > 2)
            {
                char* fileName = parts[1];
                char* fileData = parts[2];
                
                size_t dataLen = strlen(fileData);

                file_h file = ezfs_find_file(fileName);
                
                if(file != FILE_NOT_FOUND)
                {
                    uint8_t* data = NULL;
                    size_t bytes = ezfs_read_file(file, &data);
                    
                    //uint8_t* concatBuf = array_concat((uint8_t*)fileData, dataLen, data, bytes);
                    uint8_t* concatBuf = array_concat(data, bytes, (uint8_t*)fileData, dataLen);
                    
                    size_t writtenData = ezfs_write_file(file, concatBuf, dataLen + bytes);
                    
                    if(writtenData != dataLen + bytes)
                    {
                        ksh_write_line("Wrong amount of bytes was written by 'fappend'.");
                    }
                    
                    free(concatBuf);
                    free(data);
                }
                else
                {
                    ksh_write("File not found.");
                }
                
                success = TRUE;
            }
        }
        else if(strcmp(parts[0], "fdelete") == 0)
        {
            if(nb > 1)
            {
                char* fileName = parts[1];
                
                file_h file = ezfs_find_file(fileName);
                
                if(file != FILE_NOT_FOUND)
                {
                    ezfs_delete_file(file);
                    
                    ksh_write("File deleted.");
                }
                else
                {
                    ksh_write("File not found.");
                }
                
                success = TRUE;
            }
        }
    }
    
    splfree(parts, nb);
    
    if(success == FALSE)
    {
        ksh_write_line("Unrecognized command");
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
