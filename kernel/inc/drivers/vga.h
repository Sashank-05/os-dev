#ifndef VGA13H_H
#define VGA13H_H

#define VGA13H_START 0xA0000
#define VGA13H_WIDTH 320
#define VGA13H_HEIGHT 200

void vga13h_clear(unsigned char color);
void vga13h_putpixel(int x, int y, unsigned char color);
void vga13h_draw_rect(int x, int y, int width, int height, unsigned char color);
void vga13h_draw_line(int x1, int y1, int x2, int y2, unsigned char color);

#endif