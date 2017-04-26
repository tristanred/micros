#include "error.h"

#include "kernel_log.h"

extern void error();

void TemplateFault(char* msg)
{
    Debugger();
    
    kWriteLog(msg);
    
    error();
}

void DivideByZeroFault()
{
    Debugger();
    
    kWriteLog("DIVIDE BY 0 FAULT");
    
    error();
}

void GeneralProtectionFault()
{
    Debugger();
    
    kWriteLog("GENERAL PROTECTION FAULT");    
    
    error();
}
