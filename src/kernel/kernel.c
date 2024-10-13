#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <mem.h>
#include <limine.h>
#include <kernel.h>
#include <sys.h>
#include <ssfn.h>

#define SSFN_CONSOLEBITMAP_TRUECOLOR 

extern ssfn_font_t _binary_console_sfn_start;

int main(struct limine_framebuffer *framebuffer)
{
    ssfn_src = &_binary_console_sfn_start;
    ssfn_dst.ptr = (uint8_t *)framebuffer;
    ssfn_dst.w = framebuffer->width;
    ssfn_dst.h = framebuffer->height;
    ssfn_dst.p = framebuffer->bpp / 8;
    ssfn_dst.fg = 0xffffffff;
    ssfn_dst.bg = 0xff000000;
    ssfn_dst.x = 0;
    ssfn_dst.y = 0;

    ssfn_putc('A');
    ssfn_putc('B');
    return 0;
}

