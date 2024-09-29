#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>

#define VGA_TEXT_START 0xb8000
#define VGA_TEXT_WIDTH 80
#define VGA_TEXT_HEIGHT 25

#define VGA_GRAPHICS_START 0xA0000
#define VGA_GRAPHICS_WIDTH 320
#define VGA_GRAPHICS_HEIGHT 200

#define CHAR_WIDTH 8
#define CHAR_HEIGHT 8

// VGA register ports
#define VGA_AC_INDEX 0x3C0
#define VGA_AC_WRITE 0x3C0
#define VGA_AC_READ 0x3C1
#define VGA_MISC_WRITE 0x3C2
#define VGA_SEQ_INDEX 0x3C4
#define VGA_SEQ_DATA 0x3C5
#define VGA_DAC_READ_INDEX 0x3C7
#define VGA_DAC_WRITE_INDEX 0x3C8
#define VGA_DAC_DATA 0x3C9
#define VGA_MISC_READ 0x3CC
#define VGA_GC_INDEX 0x3CE
#define VGA_GC_DATA 0x3CF
#define VGA_CRTC_INDEX 0x3D4
#define VGA_CRTC_DATA 0x3D5
#define VGA_INSTAT_READ 0x3DA

// Text mode functions
void clear_line(int line);
void clear();
void putss(char *str);
void boot_print_clear();
void boot_print(char *str);
void boot_println(char *str);
void showboot();

// Graphics mode functions
void put_pixel(int x, int y, uint8_t color);
void draw_char(int x, int y, unsigned int c, unsigned char color);
void print_string(int x, int y, const char* str, uint8_t color);
void graphics_clear(uint8_t color);
void graphics_showboot();

// VGA manipulation functions
void write_registers(uint8_t *regs);
void set_plane(uint8_t plane);
void set_mode_13h();

#endif // SCREEN_H