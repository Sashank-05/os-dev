#pragma once
#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <mem.h>
//extern struct limine_framebuffer *framebuffer;
extern struct limine_framebuffer *framebuffer;

typedef struct {
    unsigned short magic;  // Magic number (0x0436 for PSF1)
    unsigned char mode;    // Font mode (bit 0 = 512 characters if set, otherwise 256)
    unsigned char charsize; // Character height in pixels (typically 16 or 8)
} psf1_header;

extern unsigned char _binary_font_psf_start[];

int main(struct limine_framebuffer *framebuffer);

static unsigned int cursorx = 0;
static unsigned int cursory = 0;


#endif