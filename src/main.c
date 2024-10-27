#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <boot/limine.h>
#include <global.h>
#include <graphics/boot.h>
#include <boot/animate.h>
#include <cpu/gdt.h>
#include <boot/bgrt.h>

__attribute__((used, section(".requests"))) static volatile LIMINE_BASE_REVISION(2);

__attribute__((used, section(".requests"))) static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0};

__attribute__((used, section(".requests_start_marker"))) static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker"))) static volatile LIMINE_REQUESTS_END_MARKER;

struct limine_framebuffer *framebuffer;

static void hcf(void)
{
    for (;;)
    {
        asm("hlt");
    }
}

void kmain(void)
{
    int frame = 0;
    if (LIMINE_BASE_REVISION_SUPPORTED == false)
    {
        hcf();
    }

    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1)
    {
        hcf();
    }

    framebuffer = framebuffer_request.response->framebuffers[0];

    if (init_graphics() == ERR_INVALID_FRAMEBUFFER || init_graphics() == ERR_UNSUPPORTED_BPP){
        // beep 

    }

    gdt_init();

    display_vendor_logo();
    while (true)
    {
        draw_frame(&frame);
        for (int i = 0; i < 100000000; i++)
        {
            asm("nop");
        }
    }
    hcf();
}