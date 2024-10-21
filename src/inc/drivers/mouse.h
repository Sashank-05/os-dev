#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <kernel.h>
#include <isr.h>

#define CURSOR_WIDTH 16
#define CURSOR_HEIGHT 16

struct mouse_state {
    int x;
    int y;
    int z;
    bool left;
    bool right;
    bool middle;
};

// Function declarations
void mouse_callback(registers_t *regs);
uint8_t mouse_timeout(uint8_t signal);
uint8_t mouse_read();
void mouse_write(uint8_t write);
void init_mouse();
void draw_mouse(struct mouse_state mstate, uint32_t color);

// External declarations
extern struct limine_framebuffer *framebuffer;