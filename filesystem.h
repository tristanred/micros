#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "fs_types.h"
#include "ezfs.h"

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

// FILE SYSTEM API

void format_disk_ezfs();

file_h create_file(file_h dir, char* name, enum FS_FILE_ACCESS access, enum FS_FILE_FLAGS flags);
file_h find_file(file_h file, char* name);
size_t read_file(file_h file, uint8_t* buf);
size_t write_file(file_h file, uint8_t* buf, size_t bufLen);
void rename_file(file_h file, char* toName);
void protect_file(file_h file, enum FS_FILE_ACCESS access);
void delete_file(file_h file);


#endif
