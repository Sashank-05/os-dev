#pragma once

#include <util.h>
#include <stdint.h>

#define MAX_TIMERS 255

void init_hardware_timer(uint32_t freq);

int add_timer(uint32_t freq_in_ticks, void (*callback)(void));

int remove_timer(void (*callback)(void));

