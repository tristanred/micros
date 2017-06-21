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

double ceil(double x)
{
    int intRoundsDown = (int)x;
    
    if(x == (double)intRoundsDown)
    {
        return intRoundsDown;
    }
    else
    {
        return ((double)intRoundsDown) + 1;
    }
}

double floor(double x)
{
    return (double)((int)x);
}
