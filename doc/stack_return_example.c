struct regs_t
{
    unsigned int eip, cs, flags; // Added eip and cs
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax; // Added by PUSHA
};


struct regs_t doStuff();
struct regs_t doStuff()
{
    //int x = 0;
    //int y = 0;
    //int z = 0;
    struct regs_t val;
    val.eip = 0;
    val.cs = 1;
    val.flags = 2;
    val.edi = 3;
    val.esi = 4;
    val.ebp = 5;
    val.esp = 6;
    val.ebx = 7;
    val.edx = 8;
    val.ecx = 9;
    val.eax = 10;

    return val;
}

void callDoStuff()
{
    struct regs_t str;
    str = doStuff();
}