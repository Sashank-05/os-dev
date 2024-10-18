#include <util.h>
#include <boot_print.h>
#include <kernel.h>
#include <io.h>
#include <idt.h>
#include <isr.h>
#include <stdint.h>

const char *sc_name[] = {"ERROR", "Esc", "1", "2", "3", "4", "5", "6",
                         "7", "8", "9", "0", "-", "=", "Backspace", "Tab", "Q", "W", "E",
                         "R", "T", "Y", "U", "I", "O", "P", "[", "]", "Enter", "Lctrl",
                         "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "`",
                         "LShift", "\\", "Z", "X", "C", "V", "B", "N", "M", ",", ".",
                         "/", "RShift", "Keypad *", "LAlt", "Spacebar"};
const char sc_ascii[] = {'?', '?', '1', '2', '3', '4', '5', '6',
                         '7', '8', '9', '0', '-', '=', '?', '?', 'Q', 'W', 'E', 'R', 'T', 'Y',
                         'U', 'I', 'O', 'P', '[', ']', '?', '?', 'A', 'S', 'D', 'F', 'G',
                         'H', 'J', 'K', 'L', ';', '\'', '`', '?', '\\', 'Z', 'X', 'C', 'V',
                         'B', 'N', 'M', ',', '.', '/', '?', '?', '?', ' '};

static void kb_callback(registers_t *regs)
{
    uint8_t scancode = inb(0x60);
    if (scancode <= 57){
        if (scancode == 28)
        {
            print_string(&cursorx, &cursory, "\n", 0xFFFFFF);
        }
        else
        {
            char str[2] = {sc_ascii[scancode], '\0'};
            print_string(&cursorx, &cursory, str, 0xFFFFFF);
        }
        //print_string(&cursorx, &cursory, sc_name[scancode], 0xFFFFFF);
    }
    

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
