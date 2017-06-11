#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"

// ***** EASY API *****
// The easy API is a bunch of functions designed to get the basic functionality
// running. After that, a more standard C interface will come

struct filesystem_info
{
    BOOL FilesystemReady;
};
struct filesystem_info* filesystem;

void setup_filesystem();

uint8_t* read_data(uint64_t startAddress, uint64_t length);

void write_data(uint8_t* data, uint64_t length, uint64_t startAddress);

// Private methods

uint64_t get_sector_from_address(uint64_t address, uint8_t* sectorOffset);

// TODO : I/O api using the FILE* c std

#endif
