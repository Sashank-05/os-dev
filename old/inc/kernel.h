#ifndef KERNEL_H
#define KERNEL_H
#include <stdint.h>

#define NULL ((void*) 0)

typedef struct {
    uint32_t type;
    uint32_t size;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
    uint16_t reserved;
} multiboot2_tag_framebuffer;

typedef struct {
    uint32_t total_size;
    uint32_t reserved;
    uint8_t tags[0];
} multiboot2_info;

unsigned int g_multiboot_info_address;

uint32_t* framebuffer;
// in characters, not pixels
uint32_t VGA_WIDTH;
uint32_t VGA_HEIGHT;

uint32_t bpp;
uint32_t pitch;
#endif
