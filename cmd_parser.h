#ifndef CMD_PARSER_H
#define CMD_PARSER_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"

enum cmdparser_state {
    CMDP_START,
    CMDP_SCAN,
    CMDP_SWITCH_START,
    CMDP_SWITCH_SCAN_NAME,
    CMDP_SWITCH_SCAN_VALUE,
    CMDP_SWITCH_END,
    
    CMDP_PLAIN_START,
    CMDP_PLAIN_SCAN,
    CMDP_PLAIN_END,
    
    CMDP_DONE
};

struct commandline_parser {
    char* commandlineString;
    size_t currentIndex;
    
    enum cmdparser_state current_state;
   
    int arglistIndex; // 
    char** arglist;
    
    int currentArgIndex;
    
    BOOL IsDone;
};

struct commandline_parser make_commandline_parser(char* commandline);

char** parse_commandline(char* cmdstring, int* count);

void process_commandline_states(struct commandline_parser* parser);

BOOL commandline_out_of_bounds(struct commandline_parser* parser);

void commandline_append_next_char(struct commandline_parser* parser);

#endif
