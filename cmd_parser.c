#include "cmd_parser.h"

#include "string.h"
#include "memory.h"

struct commandline_parser make_commandline_parser(char* commandline)
{
    struct commandline_parser newParser;
    newParser.current_state = CMDP_START;
    newParser.commandlineString = commandline;
    newParser.currentIndex = 0;
    
    newParser.arglistIndex = 0;
    newParser.arglist = malloc(sizeof(char*));
    newParser.currentArgIndex = 0;
    
    newParser.IsDone = FALSE;
    
    return newParser;
}

char** parse_commandline(char* cmdstring, int* count)
{
    (void)count;
    
    struct commandline_parser parser = make_commandline_parser(cmdstring);
    
    while(parser.IsDone == FALSE)
    {
        process_commandline_states(&parser);
    }
    
    *count = parser.arglistIndex + 1;
    
    return parser.arglist;
}

void process_commandline_states(struct commandline_parser* parser)
{
    switch(parser->current_state)
    {
        case CMDP_START:
        {
            parser->current_state = CMDP_SCAN;
            parser->IsDone = FALSE;
            parser->currentIndex = 0;
            
            break;
        }
        case CMDP_SCAN:
        {
            if(commandline_out_of_bounds(parser))
            {
                parser->current_state = CMDP_DONE;
                break;
            }
            
            if(parser->commandlineString[parser->currentIndex] == '-')
            {
                parser->current_state = CMDP_SWITCH_START;
            }
            else if(parser->commandlineString[parser->currentIndex] != ' ')
            {
                parser->current_state = CMDP_PLAIN_START;
            }
            else
            {
                parser->currentIndex++;
            }
            
            break;
        }
        case CMDP_SWITCH_START:
        {
            //parser->arglistIndex++; // Increment at the end of an arg scan ?
            parser->arglist[parser->arglistIndex] = malloc(sizeof(char) * 256);
            
            parser->currentArgIndex = 0;
            
            parser->current_state = CMDP_SWITCH_SCAN_NAME;
            
            break;
        }
        case CMDP_SWITCH_SCAN_NAME:
        {
            commandline_append_next_char(parser);
            
            if(commandline_out_of_bounds(parser))
            {
                parser->current_state = CMDP_DONE;
                break;
            }

            // Found the end of the name
            if(parser->commandlineString[parser->currentIndex] == ' ')
            {
                parser->current_state = CMDP_SWITCH_SCAN_VALUE;
            }
            
            break;
        }
        case CMDP_SWITCH_SCAN_VALUE:
        {
            commandline_append_next_char(parser);
            
            if(commandline_out_of_bounds(parser))
            {
                parser->current_state = CMDP_DONE;
                break;
            }

            // Found the end of the value
            if(parser->commandlineString[parser->currentIndex] == ' ')
            {
                parser->current_state = CMDP_SWITCH_END;
            }

            break;
        }
        case CMDP_SWITCH_END:
        {
            // Place a null terminator at the end of the extracted argument.
            // No need to re-increment currentArgIndex since it's going to be
            // reset to 0 at the next pass.
            parser->arglist[parser->arglistIndex][parser->currentArgIndex] = '\0';
            
            parser->arglistIndex++;
            
            parser->current_state = CMDP_SCAN;
            
            break;
        }
        case CMDP_PLAIN_START:
        {
            parser->arglist[parser->arglistIndex] = malloc(sizeof(char) * 256);
            
            parser->currentArgIndex = 0;
            
            parser->current_state = CMDP_PLAIN_SCAN;

            break;
        }
        case CMDP_PLAIN_SCAN:
        {
            commandline_append_next_char(parser);
            
            if(commandline_out_of_bounds(parser))
            {
                parser->current_state = CMDP_DONE;
                break;
            }

            // Found the end of the value
            if(parser->commandlineString[parser->currentIndex] == ' ')
            {
                parser->current_state = CMDP_PLAIN_END;
            }

            break;
        }
        case CMDP_PLAIN_END:
        {
            parser->arglistIndex++;
            
            parser->current_state = CMDP_SCAN;

            break;
        }
        case CMDP_DONE:
        {
            parser->IsDone = TRUE;
            
            break;
        }
        default:
        {
            break;
        }
    }
}

BOOL commandline_out_of_bounds(struct commandline_parser* parser)
{
    return parser->currentIndex >= strlen(parser->commandlineString);
}

void commandline_append_next_char(struct commandline_parser* parser)
{
    parser->arglist[parser->arglistIndex][parser->currentArgIndex] = parser->commandlineString[parser->currentIndex];
    parser->currentIndex++;
    parser->currentArgIndex++;
}
