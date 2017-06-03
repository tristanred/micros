#ifndef COMMON_H
#define COMMON_H

#define TRUE 1
#define FALSE 0

#define ASSERT(c, m) if((c) == FALSE) Debugger()

typedef int BOOL;

// Dummy function that gdb breaks into
void Debugger();

#endif
