#ifndef ERROR_H
#define ERROR_H

#include "types.h"

BOOL panic;

#define ASSERT(c, m) if((c) == FALSE) PanicQuit(m)

// Dummy function that gdb breaks into
void Debugger();

void PanicQuit(char* message);

void ShowErrorMessage(char* errorType, char* message);

//
void TemplateFault(char* msg);

void DivideByZeroFault();
void GeneralProtectionFault();

#endif
