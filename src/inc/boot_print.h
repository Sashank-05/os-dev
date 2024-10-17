#ifndef PRINT_H
#define PRINT_H

#include <stdint.h>
#include <stddef.h>
#include <kernel.h>
#include <limine.h>

extern struct limine_framebuffer *framebuffer;


extern unsigned int cursorx;
extern unsigned int cursory;

void init_back_framebuffer();

void print_string(unsigned int *x, unsigned int *y, const char* s, uint32_t color);
void print_string_noptr(unsigned int x, unsigned int y, const char* s, uint32_t color);
void draw_char(unsigned int x, unsigned int y, unsigned char c, uint32_t color);
void draw_pixel(int x, int y, uint32_t color);
void clearscreen(uint32_t color);


#endif