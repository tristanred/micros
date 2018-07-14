#include "kernel_log.h"

#include "string.h"

void kSetupLog(COMPRT loggingPort)
{
    keLogPort = seSetupCOMPort(loggingPort);
}

void kWriteLog(const char* str, ...)
{
    if(keLogPort != 0)
    {
        char outbuf[COMSTRMAX];
        
        va_list formats;
        va_start(formats, str);
        vsprintf(outbuf, str, formats);
        va_end(formats);
        
        seWriteString(keLogPort, outbuf);
        seWriteByte(keLogPort, '\n');
    }
}
