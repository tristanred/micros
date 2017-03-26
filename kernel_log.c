#include "kernel_log.h"

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
