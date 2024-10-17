#pragma once 

#include <util.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <mem.h>
#include <kernel.h>
#include <boot_print.h>
#include <idt.h>
#include <isr.h>

struct mouse_state
{
    int x;
    int y;
    int z;
    bool left;
    bool right;
    bool middle;
};

void mouse_callback(registers_t *regs);
uint8_t mouse_timeout(uint8_t signal);
uint8_t mouse_read();
void mouse_write(uint8_t write);
void init_mouse();



