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
    
    CMDP_PLAIN_START,
    CMDP_PLAIN_SCAN,
    
    CMDP_DONE
};

struct commandline_parser {
    char* commandlineString;
    size_t currentIndex;
    
    enum cmdparser_state current_state;
    
    int arglistIndex;
    int currentArgIndex;
    char** arglist;
    
    BOOL IsDone;
};

struct commandline_parser make_commandline_parser(char* commandline);

void process_commandline(struct commandline_parser* parser);

#endif
