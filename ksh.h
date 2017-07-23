#ifndef KSH_H
#define KSH_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "keyboard.h"
#include "framebuffer.h"

#define KSH_MAX_LINE_LENGTH 80
#define KSH_LINES_IN_MEMORY FBROWS

char** lines;
//int linesCount; // linesCount -1 is the current typing line.

int cursorColumn;

char* promptText;
int promptLength;

BOOL ksh_take_fb_control();

void ksh_fb_release();

void ksh_update();

//void ksh_write_line(char* line);

void ksh_render_line(int nb);

void ksh_type_character(char value);

void ksh_erase_character();

void ksh_enter_command();

char* ksh_get_current_type_line();
BOOL ksh_is_current_type_line(int nb);
int ksh_get_current_line_nb();

void ksh_push_lines();

void ksh_kb_hook(keyevent_info* info);

#endif
