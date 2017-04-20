#ifndef TIMER_H
#define TIMER_H

#include <stddef.h>
#include <stdint.h>

uint32_t timer_freq;
uint32_t timer_div;

uint32_t mscounter;

void init_timer(uint32_t frequency);

#endif
