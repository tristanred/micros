#ifndef GDT_H
#define GDT_H

#include <stdint.h>

/*
 * Segment Descriptor
 * Intel Vol. 3A 3-10
 *
 * HIGH DWORD
 * | 31 .. 24 | 23 | 22 | 21 | 20 | 19 .. 16
 * |   hBase  | G  |DIB | L  |AVL |  hLimit
 *
 * | 15 | 14 13 | 12 | 11 .. 8 | 7 .. 0
 * | P  |  DPL  | S  |   TYPE  |  mBase
 *
 * LOW DWORD
 * | 31 .. 16 |
 * |  lBase   |
 * | 15 .. 0  |
 * |  lLimit  |
 */
typedef struct {
   uint16_t           lLimit;
   uint16_t           lBase;
   uint8_t            mBase;
   uint8_t            access;
   uint8_t            granularity;
   uint8_t            hBase;
} __attribute__((packed)) gdtEntry_t;

struct {
   uint16_t limit;
   uint32_t base;
} __attribute__((packed)) gdtPointer;

extern void loadGdt();

void setupGdt();




#endif
