#include "kernel_log.h"

#include "string.h"

void kSetupLog(COMPRT loggingPort)
{
    keLogPort = seSetupCOMPort(loggingPort);
}

void kWriteLog(const char* str)
{
    if(keLogPort != 0)
    {
        seWriteString(keLogPort, str);
        seWriteByte(keLogPort, '\n');
    }
}

void kWriteLog_format1d(const char* str, uint32_t number)
{
    char outBuf[256];
    
    sprintf_1d(outBuf, str, number);
    
    kWriteLog(outBuf);
}
