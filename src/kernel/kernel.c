
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
#include <timer.h>

// extern ssfn_font_t _binary_3270_sfn_start;
extern unsigned char _binary_unifont_sfn_start;

// extern char _binary_font_psf_start[];
extern char _binary_font_psf_end;

unsigned int cursorx = 0;
unsigned int cursory = 0;

int font_height;

void wait_for_rtc_update()
{
    outb(0x70, 0x0A);
    while (inb(0x71) & 0x80)
        ;
}

uint8_t read_rtc_register(int reg)
{
    outb(0x70, reg);
    return inb(0x71);
}

uint8_t bcd_to_bin(uint8_t value)
{
    return (value & 0x0F) + ((value / 16) * 10);
}

uint8_t bin_to_bcd(uint8_t value)
{
    return ((value / 10) * 16) + (value % 10);
}

void print_time() {
    wait_for_rtc_update();

    // Prepare buffers for the time values
    char seconds[3];
    char minutes[3];
    char hours[3];

    // Convert BCD to string representations
    itoa(bcd_to_bin(read_rtc_register(0x00)), seconds);
    itoa(bcd_to_bin(read_rtc_register(0x02)), minutes);
    itoa(bcd_to_bin(read_rtc_register(0x04)), hours);

    // Set cursor to the desired position
    int curcopy[] = {cursorx, cursory};
    cursorx = framebuffer->width - 500; // Adjust x position as needed
    cursory = framebuffer->height - 200; // Adjust y position as needed

    // Clear the current line by overwriting with spaces
    print_string(&cursorx, &cursory, "                                       ", 0x0F0F0F0F);  

    // Reset cursor position
    cursorx = framebuffer->width - 500; // Reset x position
    cursory = framebuffer->height - 200; // Reset y position

    // Print the time in the desired format
    print_string(&cursorx, &cursory, "Time: ", 0xFFFFFF);
    print_string(&cursorx, &cursory, hours, 0xFFFFFF);
    print_string(&cursorx, &cursory, ":", 0xFFFFFF);
    print_string(&cursorx, &cursory, minutes, 0xFFFFFF);
    print_string(&cursorx, &cursory, ":", 0xFFFFFF);
    print_string(&cursorx, &cursory, seconds, 0xFFFFFF);

    // Reset the cursor to original position
    cursorx = curcopy[0];
    cursory = curcopy[1];
}


void roundrobin()
{
    int cursorcopyx = cursorx;
    int cursorcopyy = cursory;

    cursorx = framebuffer->width - 200;
    cursory = framebuffer->height - 100;
    print_string(&cursorx, &cursory, "Round Robin\n", 0xFF0000);
    cursorx = framebuffer->width - 200;
    cursory = framebuffer->height - 100;
    print_string(&cursorx, &cursory, "Round Robin\n", 0xFFFFFF);
    cursorx = cursorcopyx;
    cursory = cursorcopyy;
}


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

    // clearscreen(0x000000);

    init_mouse();

    for (int i = 0; i < 6; i++)
    {
        print_string(&cursorx, &cursory, boot_message[i], 0xFFFFFF);
    }

    print_string(&cursorx, &cursory, "Cursor init, disabling it temporarily", 0x0F01F0);

    // disable cursor

    // raise an isr
    // asm volatile("int $0x10");

    init_hardware_timer(1000);

    // init round robin of time quantum 100ms

    add_timer(100, roundrobin);
    add_timer(500, print_time);

    return 0;
}

