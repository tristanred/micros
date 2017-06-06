#include "disk.h"
#include "io_func.h"

extern void ata_lba_read();
extern void ata_lba_write();

void disk_read()
{
    ata_lba_read();
}

void disk_write()
{
    ata_lba_write();
}
