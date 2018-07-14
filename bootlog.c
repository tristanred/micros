#include "bootlog.h"

void kBootProgress(const char* str)
{
    kWriteLog(str);
    fbPutString(str);
}
