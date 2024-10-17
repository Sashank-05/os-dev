#include <util.h>
#include <boot_print.h>
#include <kernel.h>
#include <io.h>
#include <idt.h>
#include <isr.h>
#include <stdint.h>

static void kb_callback(registers_t *regs)
{
    uint8_t scancode = inb(0x60);
    char s[21];
    itoa(scancode, s);

    if (scancode == 0x1C)
    {
        print_string(&cursorx, &cursory, "Enter", 0xFFFFFF);
        clearscreen(0x0F0F0F);
        return;
    }
    else
    {
        print_string(&cursorx, &cursory, "Keycode: ", 0xFFFFFF);
    }

    print_string(&cursorx, &cursory, s, 0xFFFFFF);
}

void kb_install()
{
    // SEND 0XF4 TO THE Kb
    outb(0x60, 0xF4);
    // READ THE ACKNOWLEDGEMENT
    uint8_t x = inb(0x60);

    if (x == 0xFA)
    {
        print_string(&cursorx, &cursory, "Keyboard ACKNOWLEDGED\n", 0xFFFFFF);
    }
    else
    {
        print_string(&cursorx, &cursory, "Keyboard NOT ACKNOWLEDGED\n", 0xFFFFFF);
    }

    // self test
    outb(0x60, 0xFF);
    uint8_t y = inb(0x60);

    while (y != 0xAA || y != 0xFA || y != 0xFE)
    {
        y = inb(0x60);

        if (y == 0xAA)
        {
            print_string(&cursorx, &cursory, "Keyboard self test passed\n", 0xFFFFFF);
            break;
        }
        else if (y == 0xFA)
        {
            print_string(&cursorx, &cursory, "Keyboard ACK\n", 0xFFFFFF);
        }
        else if (y == 0xFE)
        {
            print_string(&cursorx, &cursory, "RESEND\n", 0xFFFFFF);
        }
        else
        {
            print_string(&cursorx, &cursory, "Keyboard self test failed\n", 0xFFFFFF);
        }
        clearscreen(0x0F0F0F);
    }

    register_interrupt_handler(IRQ1, kb_callback);
}
