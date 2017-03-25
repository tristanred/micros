#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include <stddef.h>

typedef uint16_t COMPTR;

// COM 1
#define SERIAL_COM1_BASE                0x3F8
#define SERIAL_DATA_PORT(base)          (base)
#define SERIAL_FIFO_COMMAND_PORT(base)  (base + 2)
#define SERIAL_LINE_COMMAND_PORT(base)  (base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base)   (base + 5)


// PORT COMMANDS
#define SERIAL_LINE_ENABLE_DLAB         0x80

void seConfigureBaudRate(uint16_t com, uint16_t speedDivisor);
void seConfigureLine(uint16_t com);
void seConfigureBuffers(uint16_t com);
void seConfigureModel(uint16_t com);

uint16_t seCheckBufferEmpty(uint16_t com);

COMPTR seSetupCOMPort(uint16_t com);

void seWriteByte(COMPTR comPort, uint16_t data);

#endif
