#include <global.h>
#include <boot/limine.h>
extern int cursor_x;
extern int cursor_y;

int cursor_x = 0;
int cursor_y = 0;

extern unsigned char _binary_font_psf_start[];

void draw_char(unsigned int x, unsigned int y, unsigned char c, uint32_t color)
{
    psf1_header *font_header = (psf1_header *)_binary_font_psf_start;
    unsigned char *glyph = _binary_font_psf_start + sizeof(psf1_header) + (c * font_header->charsize);

    for (int i = 0; i < font_header->charsize; i++)
    {
        unsigned char row = glyph[i];
        for (int j = 0; j < 8; j++)
        {
            if (row & (1 << (7 - j)))
            {
                draw_pixel(x + j, y + i, color);
            }
        }
    }
}

void print_string(const char *str, unsigned int x, unsigned int y, uint32_t color)
{
    unsigned int original_x = x;
    psf1_header *font_header = (psf1_header *)_binary_font_psf_start;

    while (*str != 0)
    {
        if (*str == '\n')
        {
            x = 0;
            y += 16;
        }
        elif (*str == '\b')
        {
            x -= 8;
            for (int i = 0; i < font_header->charsize; i++)
            {
                for (int j = 0; j < 8; j++)
                {
                    draw_pixel(x + j, y + i, 0x00000000);
                }
            }
        }
        elif (*str == ' ')
        {
            for (int i = 0; i < font_header->charsize; i++)
            {
                for (int j = 0; j < 8; j++)
                {
                    //draw_pixel(x + j, y + i, 0x00000000);
                }
            }
            x += 8;
        }
        else
        {
            draw_char(x, y, *str, color);
            x += 8;
        }
        str++;
    }
}

void print(char *str)
{
    print_string(str, 0, 0, 0xFFFFFFFF);
}