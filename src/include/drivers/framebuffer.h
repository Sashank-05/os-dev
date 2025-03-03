#pragma once

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdarg.h>
#include <stddef.h>
#include <limine.h>
#include <stdint.h>
#include <drivers/framebuffer.h>


#define GFX_BLACK	0x00000000
#define GFX_RED		0xFFFF0000
#define GFX_GREEN	0xFF00FF00
#define GFX_YELLOW	0xFFE6D6AC
#define GFX_BLUE	0xFF0000FF
#define GFX_PURPLE	0xFFB18FCF
#define GFX_CYAN	0xFF8CD0D3
#define GFX_WHITE	0xFFFFFFFF


#define ANSI_BLACK	"\e[30m"
#define ANSI_RED	"\e[31m"
#define ANSI_GREEN	"\e[32m"
#define ANSI_YELLOW	"\e[33m"
#define ANSI_BLUE	"\e[34m"
#define ANSI_PURPLE	"\e[35m"
#define ANSI_CYAN	"\e[36m"
#define ANSI_WHITE	"\e[37m"


extern uint8_t _binary_sfn_fonts_unifont_sfn_start;


extern size_t gfx_array[7];

struct GFX_Struct
{
	uint64_t fb_addr;
	uint16_t fb_width;
	uint16_t fb_height;
	uint16_t fb_pitch;
	uint16_t fb_bpp;

	int glyph_width;
	int glyph_height;
};

void framebuffer_init(uint32_t background_color);
void framebuffer_draw_pixel(int x, int y, uint32_t color);
void framebuffer_set_background_color(uint32_t background_color);
void framebuffer_reset_screen(void);
void framebuffer_draw_line(int x_start_pos, int y_start_pos, int x_end_pos, int y_end_pos, uint32_t color);
void framebuffer_draw_circle(int x_center, int y_center, int radius, uint32_t color);
void framebuffer_move_one_row_up(void);
void framebuffer_print_char(uint32_t unicode_character, int x, int y, uint32_t foreground_color);
void framebuffer_print_string(char* string, uint32_t foreground_color);

//void printk(uint32_t color, const char* format, ...);


#endif
