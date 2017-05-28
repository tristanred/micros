#include "cmd_parser.h"

#include "string.h"
#include "memory.h"

struct commandline_parser make_commandline_parser(char* commandline)
{
    struct commandline_parser newParser;
    newParser.commandlineString = commandline;
    newParser.currentIndex = 0;
    
    newParser.arglistIndex = 0;
    newParser.arglist = malloc(sizeof(char*));
    
    newParser.IsDone = FALSE;
    
    return newParser;
}

void process_commandline(struct commandline_parser* parser)
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
            if(parser->commandlineString[parser->currentIndex] == '-')
            {
                parser->current_state = CMDP_SWITCH_SCAN_NAME;
            }
            else if(parser->commandlineString[parser->currentIndex] != ' ')
            {
                parser->current_state = CMDP_PLAIN_SCAN;
            }
            else
            {
                parser->currentIndex++;
            }
            
            break;
        }
        case CMDP_SWITCH_START:
        {
            parser->arglistIndex++;
            parser->arglist[parser->arglistIndex] = malloc(sizeof(char) * 256);
            
            parser->current_state = CMDP_SWITCH_SCAN_NAME;
            
            break;
        }
        case CMDP_SWITCH_SCAN_NAME:
        {
            parser->arglist[parser->arglistIndex][parser->currentArgIndex] = parser->commandlineString[parser->currentIndex];
            parser->currentIndex++;
            parser->currentArgIndex++;
            
            if(parser->currentIndex >= strlen(parser->commandlineString))
            {
                parser->current_state = CMDP_DONE;
                break;
            }

            if(parser->commandlineString[parser->currentIndex] == ' ')
            {
                parser->current_state = CMDP_SWITCH_SCAN_VALUE;
            }
            
            break;
        }
        case CMDP_SWITCH_SCAN_VALUE:
        {
            parser->arglist[parser->arglistIndex][parser->currentArgIndex] = parser->commandlineString[parser->currentIndex];
            parser->currentIndex++;
            parser->currentArgIndex++;
            
            if(parser->currentIndex >= strlen(parser->commandlineString))
            {
                parser->current_state = CMDP_DONE;
                break;
            }

            if(parser->commandlineString[parser->currentIndex] == ' ')
            {
                parser->current_state = CMDP_SCAN;
            }

            break;
        }
        default:
        {
            break;
        }
    }
}
