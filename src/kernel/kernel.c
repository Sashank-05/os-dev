
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <mem.h>
#include <limine.h>
#include <kernel.h>
#include <boot_print.h>
#include <gdt.h>
#include <idt.h>
#include <isr.h>
#include <drivers/kb.h>
#include <drivers/mouse.h>

// extern ssfn_font_t _binary_3270_sfn_start;
extern unsigned char _binary_unifont_sfn_start;

// extern char _binary_font_psf_start[];
extern char _binary_font_psf_end;

extern unsigned int cursorx;
extern unsigned int cursory;

int font_height;

int main(struct limine_framebuffer *framebuffer)
{
    framebuffer = framebuffer;

    init_back_framebuffer();
    

    // print_string(cursor_x, cursor_y, "Hello, World!", 0xFFFFFF);
    //  set font height to 16
    // font_height = 16;

    print_string(&cursorx, &cursory, "Hello, World!\n", 0xFFFFFF);

    print_string(&cursorx, &cursory, "Hello, World once again", 0xFFFFFF);



    clearscreen(0x0F0F0F0F);

    cursorx = 0;
    cursory = 0;

    // print SAS OS in the centre of the screen
    const char *boot_message[] = {
        "   _____               _____      ____     _____ \n",
        "  / ____|     /\\      / ____|    / __ \\   / --__|\n",
        " | (___      /  \\    | (___     | |  | | | (___  \n",
        "  \\___ \\    / /\\ \\    \\___ \\    | |  | |  \\___ \\ \n",
        "  ____) |  / ____ \\   ____) |   | |__| |  ____) |\n",
        " |_____/  /_/    \\_\\ |_____/     \\____/  |_____/ \n"};

    for (int i = 0; i < 6; i++)
    {
        print_string(&cursorx, &cursory, boot_message[i], 0xFFFFFF);
    }

    // move the cursor below the half of screen
    cursory += 8 * 6 + 12;

    // print the message "Booting the kernel..."
    print_string(&cursorx, &cursory, "Booting the kernel...\n", 0x0F01F0);

    // setup gdt

    gdt_init();

    print_string(&cursorx, &cursory, "GDT initialized!\n", 0xFFFFFF);

    // setup idt
    isr_install();

    print_string(&cursorx, &cursory, "IDT initialized!\n", 0xFFFFFF);

    asm volatile("sti");

    print_string(&cursorx, &cursory, "Interrupts enabled!\n", 0xFFFFFF);

    // setup keyboard
    kb_install();

    print_string(&cursorx, &cursory, "Keyboard initialized!\n", 0xFFFFFF);

    // nop for sometime
    for (int i = 0; i < 1000000000; i++)
    {
        asm("nop");
    }

    //clearscreen(0x000000);

    init_mouse();

    for (int i = 0; i < 6; i++)
    {
        print_string(&cursorx, &cursory, boot_message[i], 0xFFFFFF);
    }
    
    return 0;
}
