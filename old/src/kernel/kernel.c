#include <multiboot2.h>
#include <kernel.h>
#include <stdint.h>

void putpixel(uint32_t *fb, int pitch, int bpp, int x, int y, uint32_t color)
{
    uint32_t *pixel_addr = (uint32_t *)((uint8_t *)fb + y * pitch + x * (bpp / 8));
    *pixel_addr = color;
}

void main()
{
    // puts("Hello, World!");
    //  in 32 bpp
    //  print a red pixel from 100, 100 to 200, 200
    for (int y = 100; y < 200; y++)
    {
        for (int x = 100; x < 200; x++)
        {
            putpixel(framebuffer, pitch, bpp, x, y, 0xFF0000);
        }
    }
    while (1)
        ;
}

void kernel_main(multiboot2_info *mb_info)
{
    multiboot2_tag_framebuffer *fb_info = NULL;
    struct multiboot_tag_mmap *mmap_tag = NULL;
    struct multiboot_tag_module *initrd_module = NULL;

    uint8_t *tags = mb_info->tags;
    while (1)
    {
        uint32_t tag_type = *((uint32_t *)tags);
        uint32_t tag_size = *((uint32_t *)(tags + 4));

        if (tag_type == 0)
            break;
        if (tag_type == MULTIBOOT_TAG_TYPE_FRAMEBUFFER)
        {
            fb_info = (multiboot2_tag_framebuffer *)tags;
        }
        if (tag_type == MULTIBOOT_TAG_TYPE_MMAP)
        {
            mmap_tag = (struct multiboot_tag_mmap *)tags;
        }
        if (tag_type == MULTIBOOT_TAG_TYPE_MODULE)
        {
            initrd_module = (struct multiboot_tag_module *)tags;
        }

        tags += ((tag_size + 7) & ~7);
    }

    if (fb_info)
    { // fb setup
        framebuffer = (uint32_t *)(uintptr_t)fb_info->framebuffer_addr;

        uint32_t width = fb_info->framebuffer_width;
        uint32_t height = fb_info->framebuffer_height;
        bpp = fb_info->framebuffer_bpp;
        pitch = fb_info->framebuffer_pitch;

        // 8x16 font, not padded
        VGA_WIDTH = width / 8;
        VGA_HEIGHT = height / 16;

        main();
    }
}
