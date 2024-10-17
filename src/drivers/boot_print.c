#include <stdint.h>
#include <stddef.h> 
#include <limine.h> 
#include <kernel.h>
#include <boot_print.h>


struct limine_framebuffer *linearframebuffer;

void init_back_framebuffer() {
    linearframebuffer = framebuffer;
}

void swap_framebuffer() {
    struct limine_framebuffer *temp = framebuffer;
    framebuffer = linearframebuffer;
    linearframebuffer = temp;
    //print_string(&cursorx, &cursory, "\nFB swapped\n", 0xFFFFFF);
}

void draw_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && (uint64_t)x < linearframebuffer->width && y >= 0 && (uint64_t)y < linearframebuffer->height) {
        uint32_t *pixel_address = (uint32_t *)((uintptr_t)linearframebuffer->address + (y * linearframebuffer->pitch + x * sizeof(uint32_t)));
        *pixel_address = color;
    }
}

void draw_char(unsigned int x, unsigned int y, unsigned char c, uint32_t color) {
    psf1_header* font_header = (psf1_header*)_binary_font_psf_start;
    unsigned char* glyph = _binary_font_psf_start + sizeof(psf1_header) + (c * font_header->charsize);

    for (int i = 0; i < font_header->charsize; i++) {
        unsigned char row = glyph[i];
        for (int j = 0; j < 8; j++) {
            if (row & (1 << (7 - j))) {
                draw_pixel(x + j, y + i, color);
            }
        }
    }
    
}

void print_string(unsigned int *x, unsigned int *y, const char* s, uint32_t color) {
    while (*s) {
        if (*s == '\n') {
            *x = 0;
            *y += 16;
            s++;
            continue;
        }

        if (*x + 8 > linearframebuffer->width) {
            *x = 0;
            *y += 16;
        }

        draw_char(*x, *y, *s, color);
        *x += 8;
        s++;
    }
    swap_framebuffer();
}

void print_string_noptr(unsigned int x, unsigned int y, const char* s, uint32_t color) {
    while (*s) {
        draw_char(x, y, *s, color);
        x += 8;
        s++;
    }
    swap_framebuffer();
}

void clearscreen(uint32_t color) {
    for (unsigned int i = 0; i < linearframebuffer->width; i++) {
        for (unsigned int j = 0; j < linearframebuffer->height; j++) {
            draw_pixel(i, j, color);
        }
    }
    swap_framebuffer();
}
