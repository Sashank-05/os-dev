#include <drivers/framebuffer.h>

#include <common/stdinfo.h>

#define SSFN_CONSOLEBITMAP_TRUECOLOR
#define SSFN_NOIMPLEMENTATION
#include <ssfn.h>

#include <libk/stdio.h>
#include <libk/string.h>


struct GFX_Struct gfx;

extern struct OSinfo osinfo;

size_t gfx_array[] = { GFX_BLACK, GFX_RED, GFX_GREEN, GFX_YELLOW, GFX_BLUE, GFX_PURPLE, GFX_CYAN, GFX_WHITE };



void framebuffer_init(uint32_t background_color)
{
	if (osinfo.framebuffer == NULL || osinfo.framebuffer->framebuffer_count < 1) {
		return;
	}

	struct limine_framebuffer* framebuffer_info = osinfo.framebuffer->framebuffers[0];

	gfx.fb_addr = (uint64_t)framebuffer_info->address;
	gfx.fb_width = framebuffer_info->width;
	gfx.fb_height = framebuffer_info->height;
	gfx.fb_pitch = framebuffer_info->pitch;
	gfx.fb_bpp = framebuffer_info->bpp;
	gfx.glyph_width = 8;
	gfx.glyph_height = 16;

	ssfn_src = (ssfn_font_t*)&_binary_sfn_fonts_unifont_sfn_start;
	ssfn_dst.ptr = (uint8_t*)gfx.fb_addr;
	ssfn_dst.w = gfx.fb_width;
	ssfn_dst.h = gfx.fb_height;
	ssfn_dst.p = gfx.fb_pitch;
	ssfn_dst.x = 0;
	ssfn_dst.y = 0;

	framebuffer_set_background_color(background_color);
}

inline void framebuffer_draw_pixel(int x, int y, uint32_t color) {
	((uint32_t*)gfx.fb_addr)[y * (gfx.fb_pitch / sizeof(uint32_t)) + x] = color;
}

void framebuffer_set_background_color(uint32_t background_color)
{
	ssfn_dst.bg = background_color;

	for (int y = 0; y < gfx.fb_height; y++)
	{
		for (int x = 0; x < gfx.fb_width; x++)
			framebuffer_draw_pixel(x, y, ssfn_dst.bg);
	}
}

void framebuffer_reset_screen(void)
{
	ssfn_dst.x = 0;
	ssfn_dst.y = 0;

	framebuffer_set_background_color(ssfn_dst.bg);
}

void framebuffer_draw_line(int x_start_pos, int y_start_pos, int x_end_pos, int y_end_pos, uint32_t color)
{
	int dx;
	int dy;
	int sx;
	int sy;
	int error1;
	int error2;

	if (x_start_pos == x_end_pos)
	{
		if (y_start_pos < y_end_pos)
			for (int y = y_start_pos; y <= y_end_pos; y++)
				framebuffer_draw_pixel(x_start_pos, y, color);
		else
			for (int y = y_end_pos; y <= y_start_pos; y++)
				framebuffer_draw_pixel(x_start_pos, y, color);

		return;
	}

	if (y_start_pos == y_end_pos)
	{
		if (x_start_pos < x_end_pos)
			for (int x = x_start_pos; x <= x_end_pos; x++)
				framebuffer_draw_pixel(x, y_start_pos, color);
		else
			for (int x = x_end_pos; x <= x_start_pos; x++)
				framebuffer_draw_pixel(x, y_start_pos, color);

		return;
	}

	if (x_start_pos < x_end_pos)
	{
		dx = x_end_pos - x_start_pos;
		sx = 1;
	}
	else
	{
		dx = x_start_pos - x_end_pos;
		sx = -1;
	}

	if (y_start_pos < y_end_pos)
	{
		dy = -(y_end_pos - y_start_pos);
		sy = 1;
	}
	else
	{
		dy = -(y_start_pos - y_end_pos);
		sy = -1;
	}

	error1 = dx + dy;

	framebuffer_draw_pixel(x_start_pos, y_start_pos, color);

	while (x_start_pos != x_end_pos && y_start_pos != y_end_pos)
	{
		error2 = 2 * error1;

		if (error2 >= dy)
		{
			error1 += dy;
			x_start_pos += sx;
		}

		if (error2 <= dx)
		{
			error1 += dx;
			y_start_pos += sy;
		}

		framebuffer_draw_pixel(x_start_pos, y_start_pos, color);
	}
}

void draw_circle_helper(int xc, int yc, int x, int y, uint32_t color)
{
	framebuffer_draw_pixel(xc + x, yc + y, color);
	framebuffer_draw_pixel(xc - x, yc + y, color);
	framebuffer_draw_pixel(xc + x, yc - y, color);
	framebuffer_draw_pixel(xc - x, yc - y, color);
	framebuffer_draw_pixel(xc + y, yc + x, color);
	framebuffer_draw_pixel(xc - y, yc + x, color);
	framebuffer_draw_pixel(xc + y, yc - x, color);
	framebuffer_draw_pixel(xc - y, yc - x, color);
}

void framebuffer_draw_circle(int x_center, int y_center, int radius, uint32_t color)
{
	int x = 0;
	int y = radius;
	int d = 3 - 2 * radius;

	draw_circle_helper(x_center, y_center, x, y, color);

	while (y >= x)
	{
		x += 1;

		if (d > 0)
		{
			y -= 1;
			d = d + 4 * (x - y) + 10;
		}
		else
			d = d + 4 * x + 6;

		draw_circle_helper(x_center, y_center, x, y, color);
	}
}

void framebuffer_draw_filled_circle(int x_center, int y_center, int radius, uint32_t color) {
	int x = 0;
	int y = radius;
	int d = 3 - 2 * radius;

	while (y >= x) {
		// Draw horizontal lines using framebuffer_draw_line()
		framebuffer_draw_line(x_center - x, y_center + y, x_center + x, y_center + y, color);
		framebuffer_draw_line(x_center - x, y_center - y, x_center + x, y_center - y, color);
		framebuffer_draw_line(x_center - y, y_center + x, x_center + y, y_center + x, color);
		framebuffer_draw_line(x_center - y, y_center - x, x_center + y, y_center - x, color);

		x++;

		if (d > 0) {
			y--;
			d = d + 4 * (x - y) + 10;
		}
		else {
			d = d + 4 * x + 6;
		}
	}
}


/*void framebuffer_move_one_row_up(void)
{
	uint32_t* fb = (uint32_t*)gfx.fb_addr;

	for (int column = gfx.glyph_height; column < gfx.fb_height; column++)
	{
		for (int row = 0; row < gfx.fb_width; row++)
		{
			size_t current_index = column * (gfx.fb_pitch / sizeof(uint32_t)) + row;
			uint32_t current_color = fb[current_index];
			fb[current_index] = ssfn_dst.bg;

			size_t new_index = (column - gfx.glyph_height) * (gfx.fb_pitch / sizeof(uint32_t)) + row;
			fb[new_index] = current_color;
		}
	}
}
	*/

void framebuffer_move_one_row_up(void) {
	uint32_t* fb = (uint32_t*)gfx.fb_addr;
	const size_t row_size = gfx.fb_width * sizeof(uint32_t);
	const size_t pixels_to_clear = gfx.glyph_height * (gfx.fb_pitch / sizeof(uint32_t));
	const size_t data_size = (gfx.fb_height - gfx.glyph_height) * gfx.fb_pitch; // Total data to move

	// Move all rows up in one go
	uint32_t* src = fb + gfx.glyph_height * (gfx.fb_pitch / sizeof(uint32_t));
	uint32_t* dst = fb;
	memmove(dst, src, data_size);

	// Clear only the newly vacated rows at bottom
	uint32_t* clear_start = fb + (gfx.fb_height - gfx.glyph_height) * (gfx.fb_pitch / sizeof(uint32_t));
	for (size_t i = 0; i < pixels_to_clear; i++) {
		clear_start[i] = ssfn_dst.bg;
	}
}


void framebuffer_draw_rect(int x, int y, int width, int height, uint32_t color)
{
	// Draw horizontal lines
	framebuffer_draw_line(x, y, x + width, y, color);                     // Top line
	framebuffer_draw_line(x, y + height, x + width, y + height, color);   // Bottom line

	// Draw vertical lines
	framebuffer_draw_line(x, y, x, y + height, color);                    // Left line
	framebuffer_draw_line(x + width, y, x + width, y + height, color);    // Right line
}

void framebuffer_print_char(uint32_t unicode, int x, int y, uint32_t foreground_color)
{
	ssfn_dst.x = x;
	ssfn_dst.y = y;

	if (ssfn_dst.x + gfx.glyph_width > gfx.fb_width)
	{
		ssfn_dst.x = 0;
		ssfn_dst.y += gfx.glyph_height;
	}

	if (ssfn_dst.y >= gfx.fb_height)
	{
		ssfn_dst.x = 0;
		ssfn_dst.y = gfx.fb_height - gfx.glyph_height;
		// framebuffer_move_one_row_up();
		framebuffer_reset_screen();
	}

	if (unicode == '\t')
	{
		for (int i = 0; i < 4; i++)
			framebuffer_print_string(" ", foreground_color);

		return;
	}

	if (unicode == '\b')
	{
		if (ssfn_dst.x == 0)
		{
			if (ssfn_dst.y == 0)
				return;

			ssfn_dst.x = gfx.fb_width - gfx.glyph_width;
			ssfn_dst.y -= gfx.glyph_height;

			ssfn_putc(' ');

			ssfn_dst.x -= gfx.glyph_width;
		}
		else
		{
			ssfn_dst.x -= gfx.glyph_width;

			ssfn_putc(' ');

			ssfn_dst.x -= gfx.glyph_width;
		}

		return;
	}

	ssfn_dst.fg = foreground_color;

	ssfn_putc(unicode);
}

void framebuffer_print_string(char* string, uint32_t foreground_color)
{
	while (*string)
		framebuffer_print_char(ssfn_utf8(&string), ssfn_dst.x, ssfn_dst.y, foreground_color);
}


void printk(uint32_t color, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	char buffer[1024];
	vsnprintf(buffer, 1024, format, args);
	framebuffer_print_string(buffer, color);
	va_end(args);
}