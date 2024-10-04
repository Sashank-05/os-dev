#include <drivers/vga.h>

void vga13h_clear(unsigned char color)
{
    unsigned char *vga_memory = (unsigned char *)VGA13H_START;
    for (int i = 0; i < VGA13H_WIDTH * VGA13H_HEIGHT; i++)
    {
        vga_memory[i] = color;
    }
}

void vga13h_putpixel(int x, int y, unsigned char color)
{
    if (x >= 0 && x < VGA13H_WIDTH && y >= 0 && y < VGA13H_HEIGHT)
    {
        unsigned char *vga_memory = (unsigned char *)VGA13H_START;
        vga_memory[y * VGA13H_WIDTH + x] = color;
    }
}

void vga13h_draw_rect(int x, int y, int width, int height, unsigned char color)
{
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            vga13h_putpixel(x + j, y + i, color);
        }
    }
}

void vga13h_draw_line(int x1, int y1, int x2, int y2, unsigned char color)
{
    int dx = (x2 - x1) >= 0 ? (x2 - x1) : -(x2 - x1);
    int sx = x1 < x2 ? 1 : -1;
    int dy = (y2 - y1) >= 0 ? -(y2 - y1) : (y1 - y2);
    int sy = y1 < y2 ? 1 : -1;

    int err = dx + dy, e2;

    while (1)
    {
        vga13h_putpixel(x1, y1, color);
        if (x1 == x2 && y1 == y2)
            break;
        e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x1 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y1 += sy;
        }
    }
}