#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "ext4.h"

// ***** EASY API *****
// The easy API is a bunch of functions designed to get the basic functionality
// running. After that, a more standard C interface will come

struct filesystem_info
{
    BOOL FilesystemReady;
};
struct filesystem_info* filesystem;


void setup_filesystem();

void create_test_file();

// TODO : I/O api using the FILE* c std

#endif
