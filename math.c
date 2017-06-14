#include "math.h"

unsigned long int ulmin(unsigned long int x, unsigned long int y)
{
    if(x > y)
    {
        return y;
    }
    
    return x;
}

unsigned long int ulmax(unsigned long int x, unsigned long int y)
{
    if(x > y)
    {
        return x;
    }
    
    return y;
}
