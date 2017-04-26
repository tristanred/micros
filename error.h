#ifndef ERROR_H
#define ERROR_H

#include "common.h"

BOOL panic;

//
void TemplateFault(char* msg);

void DivideByZeroFault();
void GeneralProtectionFault();

#endif
