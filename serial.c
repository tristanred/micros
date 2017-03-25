#include "serial.h"

#include "io_func.h"

void seConfigureBaudRate(uint16_t com, uint16_t speedDivisor)
{
    outb(SERIAL_LINE_COMMAND_PORT(com), SERIAL_LINE_ENABLE_DLAB);
    outb(SERIAL_DATA_PORT(com), (speedDivisor >> 8) & 0x00FF);
    outb(SERIAL_DATA_PORT(com), speedDivisor & 0x00FF);
}

/*
 * Line config byte
 * Bit:     | 7 | 6 | 5 4 3 | 2 | 1 0 |
 * Content: | d | b | prty  | s | dl  |
 */
void seConfigureLine(uint16_t com)
{
    outb(SERIAL_LINE_COMMAND_PORT(com), 0x03);
}

/*
 * Buffer config byte
 *
 * Bit:     | 7 6 | 5  | 4 | 3   | 2   | 1   | 0 |
 * Content: | lvl | bs | r | dma | clt | clr | e |
 */
void seConfigureBuffers(uint16_t com)
{
    outb(SERIAL_LINE_COMMAND_PORT(com), 0xc7);
}

/*
 * Modem config byte
 *
 * Bit:     | 7 | 6 | 5  | 4  | 3   | 2   | 1   | 0   |
 * Content: | r | r | af | lb | ao2 | ao1 | rts | dtr |
 */
void seConfigureModem(uint16_t com)
{
    outb(SERIAL_LINE_COMMAND_PORT(com), 0x03);
}

uint16_t seCheckBufferEmpty(uint16_t com)
{
    return inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
}

COMPTR seSetupCOMPort(uint16_t com)
{
    seConfigureBaudRate(com, 1);
    seConfigureLine(com);
    seConfigureBuffers(com);
    seConfigureModem(com);
    
    return (COMPTR)com;
}

void seWriteByte(COMPTR comPort, uint16_t data)
{
    while(seCheckBufferEmpty((uint16_t)comPort) == 1)
    {
        /* Wait as long as buffer is non-enpty */
    }
    
    outb((uint16_t)comPort, data);
}
