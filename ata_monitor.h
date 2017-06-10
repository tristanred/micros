#ifndef ATA_MONITOR_H
#define ATA_MONITOR_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"

void test_io_port();

void read_bytes(int count, unsigned char* buffer);

void io_wait();

void wait_for_ready();

unsigned char get_status();

#endif
