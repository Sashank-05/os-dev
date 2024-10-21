#include <drivers/mouse.h>
#include <stddef.h>
#include <string.h>
#include <boot_print.h>
#include <idt.h>
#include <isr.h>
#include <io.h>

// Global variables
static uint8_t cycle = 0;
static struct mouse_state mstate = {0, 0, 0, false, false, false};
static uint32_t cursor_backup[CURSOR_HEIGHT][CURSOR_WIDTH];

int cursor_bitmap[CURSOR_HEIGHT][CURSOR_WIDTH] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0},
    {0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0},
    {0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0},
    {0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

// Function implementations
static void restore_cursor_background()
{
    uint32_t *fb = (uint32_t *)framebuffer->address;
    for (int y = 0; y < CURSOR_HEIGHT; y++)
    {
        for (int x = 0; x < CURSOR_WIDTH; x++)
        {
            int px = mstate.x + x;
            int py = mstate.y + y;
            if (px >= 0 && px < framebuffer->width && py >= 0 && py < framebuffer->height)
            {
                fb[py * framebuffer->width + px] = cursor_backup[y][x];
            }
        }
    }
}

void mouse_callback(registers_t *regs)
{
    (void)regs;
    uint8_t status = inb(0x64);
    while (status & 0x01)
    {
        int8_t mouse_in = inb(0x60);
        int8_t mouse_byte[3];
        if (status & 0x20)
        {
            switch (cycle)
            {
            case 0:
                mouse_byte[0] = mouse_in;
                if (!(mouse_in & 0x08))
                    return;
                cycle++;
                break;
            case 1:
                mouse_byte[1] = mouse_in;
                cycle++;
                break;
            case 2:
                mouse_byte[2] = mouse_in;
                if (mouse_byte[0] & 0x80 || mouse_byte[0] & 0x40)
                    return; // overflow packet

                restore_cursor_background();

                // Update mouse state
                mstate.x -= mouse_byte[1];
                mstate.y += mouse_byte[2];

                // Clamp mouse position to screen boundaries
                if (mstate.x < 0)
                    mstate.x = 0;
                if (mstate.x >= framebuffer->width)
                    mstate.x = framebuffer->width - 1;
                if (mstate.y < 0)
                    mstate.y = 0;
                if (mstate.y >= framebuffer->height)
                    mstate.y = framebuffer->height - 1;

                mstate.left = (mouse_byte[0] & 0x01) != 0;
                mstate.right = (mouse_byte[0] & 0x02) != 0;
                mstate.middle = (mouse_byte[0] & 0x04) != 0;

                cycle = 0;
                draw_mouse(mstate, 0xFFFFFF);
                break;
            }
        }
        status = inb(0x64);
    }
    irq_ack(12);
}

void draw_mouse(struct mouse_state mstate, uint32_t color)
{
    uint32_t *fb = (uint32_t *)framebuffer->address;
    uint32_t outline_color = 0x00000000; // Black outline
    uint32_t fill_color = 0xFFFFFFFF;    // White fill

    // Save the background
    for (int y = 0; y < CURSOR_HEIGHT; y++)
    {
        for (int x = 0; x < CURSOR_WIDTH; x++)
        {
            int px = mstate.x + x;
            int py = mstate.y + y;
            if (px >= 0 && px < framebuffer->width && py >= 0 && py < framebuffer->height)
            {
                cursor_backup[y][x] = fb[py * framebuffer->width + px];
            }
        }
    }

    // Draw the cursor

    for (int y = 0; y < CURSOR_HEIGHT; y++)
    {
        for (int x = 0; x < CURSOR_WIDTH; x++)
        {
            int px = mstate.x + x;
            int py = mstate.y + y;
            if (px >= 0 && px < framebuffer->width && py >= 0 && py < framebuffer->height)
            {
                if (cursor_bitmap[y][x] == 1)
                {
                    fb[py * framebuffer->width + px] = fill_color;
                }
                else
                {
                    fb[py * framebuffer->width + px] = outline_color;
                }
            }
        }
    }
}

uint8_t mouse_timeout(uint8_t signal)
{
    unsigned int timeout = 100000;
    while (timeout--)
    {
        if (signal == 1)
        {
            if (inb(0x64) & 0x01)
            {
                return 1;
            }
        }
        else
        {
            if (!(inb(0x64) & 0x02))
            {
                return 1;
            }
        }
    }
    return 0; // Timeout occurred
}

uint8_t mouse_read()
{
    mouse_timeout(0);
    return inb(0x60);
}

void mouse_write(uint8_t write)
{
    mouse_timeout(1);
    outb(0x64, 0xD4);
    mouse_timeout(1);
    outb(0x60, write);
}

void init_mouse()
{
    register_interrupt_handler(IRQ12, mouse_callback);
    outb(0x64, 0xA8);
    uint8_t ack = mouse_timeout(1);
    if (ack == 0)
    {
        // boot_print("Mouse not connected\n");
        // return;
    }
    outb(0x64, 0x20);
    ack = mouse_timeout(0);
    if (ack == 0)
    {
        // boot_print("Mouse not connected\n");
        // return;
    }
    uint8_t status = (inb(0x60) | 2);
    mouse_timeout(1);
    outb(0x64, 0x60);
    mouse_timeout(1);
    outb(0x60, status);
    mouse_write(0xF6);
    mouse_read();
    mouse_write(0xF4);
    mouse_read();

    mstate.x = 0;
    mstate.y = 0;

    // boot_print("Mouse initialized\n");
}