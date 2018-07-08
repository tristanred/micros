#include "error.h"

#include "kernel_log.h"
#include "framebuffer.h"

extern void error();

void PanicQuit(char* message)
{
    ShowErrorMessage("PANIC", message);
    
    panic = TRUE;
    
    error();
}

void ShowErrorMessage(char* errorType, char* message)
{
    fbClear();
    fbMoveCursor(0, 0);
    fbPutString(errorType);
    fbPutString(" : ");
    fbPutString(message);
}

void WARN_PARAM(char* message)
{
    (void)message;
#if CHECK_PARAMS == 1
    kWriteLog(message);
#endif
}

void Debugger()
{
    
}

void TemplateFault(char* msg)
{
    Debugger();
    
    ShowErrorMessage("OTHER FAULT", msg);
    
    kWriteLog(msg);
    
    error();
}

void DivideByZeroFault()
{
    Debugger();
    
    ShowErrorMessage("DIV 0 FAULT", "DIVIDE BY 0 FAULT");
    
    kWriteLog("DIVIDE BY 0 FAULT");
    
    error();
}

void GeneralProtectionFault()
{
    Debugger();
    
    ShowErrorMessage("GP FAULT", "GENERAL PROTECTION FAULT");
    
    kWriteLog("GENERAL PROTECTION FAULT");
    
    error();
}
