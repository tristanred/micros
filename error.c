#include "error.h"

#include "kernel_log.h"
#include "framebuffer.h"

extern void error();

void PanicQuit(char* message)
{
    fbClear();
    fbMoveCursor(0, 0);
    fbPutString("PANIC : ");
    fbPutString(message);
    
    panic = TRUE;
    
    error();
}

void Debugger()
{
    
}

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
