#ifndef TIMER_H
#define TIMER_H

#include <stddef.h>
#include <stdint.h>

uint32_t timer_freq;
uint32_t timer_div;

uint32_t mscounter;

#define TIMER_FREQ_1MS 1193

void init_timer(uint32_t frequency);

void sleep(uint32_t t);

uint32_t getmscount();

uint32_t get_timer_rate();

#endif
