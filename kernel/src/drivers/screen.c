#include <drivers/screen.h>
#include <stdint.h>
#include <drivers/font.h>

extern int curr_line;
int curr_line = 10;

static inline void outb(uint16_t port, uint8_t val)
{
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Text mode functions
void boot_backspace()
{
    char *video_memory = (char *)VGA_TEXT_START;
    int line = curr_line;
    int i = 0;

    while (i < 80 && video_memory[(line * 80 + i) * 2] != ' ')
    {
        i++; // Find the last character in the line
    }

    if (i > 0)
    {
        i--;                                          // Move to the last printed character
        video_memory[(line * 80 + i) * 2] = ' ';      // Clear the last character by overwriting with a space
        video_memory[(line * 80 + i) * 2 + 1] = 0x0F; // Reset the attribute (white text on black background)
    }
}

void clear_line(int line)
{
    char *video_memory = (char *)VGA_TEXT_START;
    for (int i = 0; i < VGA_TEXT_WIDTH; i++)
    {
        video_memory[(line * VGA_TEXT_WIDTH + i) * 2] = ' ';
        video_memory[(line * VGA_TEXT_WIDTH + i) * 2 + 1] = 0xFF; // White text
    }
}

void clear()
{
    char *video_memory = (char *)VGA_TEXT_START;
    for (int i = 0; i < VGA_TEXT_WIDTH * VGA_TEXT_HEIGHT * 2; i += 2)
    {
        video_memory[i] = ' ';
        video_memory[i + 1] = 0xFF; // Black background
    }
}

void putss(char *str)
{
    char *video_memory = (char *)VGA_TEXT_START;
    int i = 0;
    while (*str != '\0')
    {
        video_memory[i] = *str;
        video_memory[i + 1] = 0x0F; // White text
        str++;
        i += 2;
    }
}

void boot_print_clear()
{
    char *video_memory = (char *)VGA_TEXT_START;
    for (int i = 10; i < VGA_TEXT_HEIGHT; i++)
    {
        for (int j = 0; j < VGA_TEXT_WIDTH; j++)
        {
            video_memory[(i * VGA_TEXT_WIDTH + j) * 2] = ' ';
            video_memory[(i * VGA_TEXT_WIDTH + j) * 2 + 1] = 0xFF; // White text
        }
    }
    curr_line = 10;
}

void boot_print(char *str)
{
    char *video_memory = (char *)VGA_TEXT_START;
    int i = 0;
    int line = curr_line;
    clear_line(line);
    while (*str != '\0')
    {
        video_memory[(line * VGA_TEXT_WIDTH + i) * 2] = *str;
        video_memory[(line * VGA_TEXT_WIDTH + i) * 2 + 1] = 0xF0;
        str++;
        i++;
    }
    curr_line++;
    if (curr_line >= VGA_TEXT_HEIGHT)
    {
        boot_print_clear();
        curr_line = 10;
    }
}

void boot_println(char *str)
{
    char *video_memory = (char *)VGA_TEXT_START;
    int i = 0;
    int line = curr_line; // local variable to keep track of current line
    clear_line(line);
    while (*str != '\0')
    {
        video_memory[(line * VGA_TEXT_WIDTH + i) * 2] = *str;
        video_memory[(line * VGA_TEXT_WIDTH + i) * 2 + 1] = 0xF0; // White text
        str++;
        i++;
    }
    if (curr_line >= VGA_TEXT_HEIGHT)
    {
        boot_print_clear();
        curr_line = 10;
    }
}

void showboot()
{
    char *video_memory = (char *)VGA_TEXT_START;
    const char *boot_message[] = {
        "   _____               _____      ____     _____ ",
        "  / ____|     /\\      / ____|    / ** \\   / **__|",
        " | (___      /  \\    | (___     | |  | | | (___  ",
        "  \\___ \\    / /\\ \\    \\___ \\    | |  | |  \\___ \\ ",
        "  ____) |  / ____ \\   ____) |   | |__| |  ____) |",
        " |_____/  /_/    \\_\\ |_____/     \\____/  |_____/ "};
    int start_x = (VGA_TEXT_WIDTH - 49) / 2;
    int start_y = 2;
    clear();
    for (int y = 0; y < 6; y++)
    {
        for (int x = 0; x < 49; x++)
        {
            video_memory[((start_y + y) * VGA_TEXT_WIDTH + (start_x + x)) * 2] = boot_message[y][x];
            video_memory[((start_y + y) * VGA_TEXT_WIDTH + (start_x + x)) * 2 + 1] = 0xF0; // White text on black background
        }
    }
    boot_print("[+] Booting...");
}

// Graphics mode functions

unsigned char *vga_buffer = (unsigned char *)VGA_GRAPHICS_START;

void put_pixel(int x, int y, unsigned char color)
{
    if (x >= 0 && x < VGA_GRAPHICS_WIDTH && y >= 0 && y < VGA_GRAPHICS_HEIGHT)
    {
        vga_buffer[y * VGA_GRAPHICS_WIDTH + x] = color;
    }
}

void draw_char(int x, int y, unsigned int c, unsigned char color)
{

    if (c >= 128)
    { // Ensure character index is within bounds
        c = 0;
    }

    for (int cy = 0; cy < 8; cy++)
    {                                    // Each character has 8 rows
        unsigned char row = font[c][cy]; // Access row for character 'c'
        for (int cx = 0; cx < 8; cx++)
        {
            // No need to change bit checking, just reverse how the x values are applied
            if (row & (0x80 >> cx))
            {
                put_pixel(x + (7 - cx), y + cy, color);
            }
        }
    }
}

void print_string(int x, int y, const char *str, unsigned char color)
{
    int original_x = x;
    while (*str)
    {
        if (*str == '\n')
        {
            y += CHAR_HEIGHT;
            x = original_x;
        }
        else
        {
            draw_char(x, y, *str, color);
            x += CHAR_WIDTH;
        }
        str++;
    }
}

void graphics_clear(unsigned char color)
{
    for (int i = 0; i < VGA_GRAPHICS_WIDTH * VGA_GRAPHICS_HEIGHT; i++)
    {
        vga_buffer[i] = color;
    }
}

void graphics_showboot()
{
    const char *boot_message[] = {
        "   _____               _____      ____     _____ ",
        "  / ____|     /\\      / ____|    / ** \\   / **__|",
        " | (___      /  \\    | (___     | |  | | | (___  ",
        "  \\___ \\    / /\\ \\    \\___ \\    | |  | |  \\___ \\ ",
        "  ____) |  / ____ \\   ____) |   | |__| |  ____) |",
        " |_____/  /_/    \\_\\ |_____/     \\____/  |_____/ "};

    graphics_clear(0); // Clear screen to black

    int start_x = (VGA_GRAPHICS_WIDTH - 49 * CHAR_WIDTH) / 2;
    int start_y = 20; // Adjust as needed

    for (int i = 0; i < 6; i++)
    {
        print_string(start_x, start_y + i * CHAR_HEIGHT, boot_message[i], 15); // 15 is typically white
    }

    print_string(10, 150, "[+] Booting...", 15);
}

// Mode switching functions

void write_registers(uint8_t *regs)
{
    // MISC
    outb(VGA_MISC_WRITE, *regs);
    regs++;

    // SEQ
    for (uint8_t i = 0; i < 5; i++)
    {
        outb(VGA_SEQ_INDEX, i);
        outb(VGA_SEQ_DATA, *regs);
        regs++;
    }

    // CRTC
    outb(VGA_CRTC_INDEX, 0x03);
    outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) | 0x80);
    outb(VGA_CRTC_INDEX, 0x11);
    outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) & ~0x80);

    regs[0x03] = regs[0x03] | 0x80;
    regs[0x11] = regs[0x11] & ~0x80;

    for (uint8_t i = 0; i < 25; i++)
    {
        outb(VGA_CRTC_INDEX, i);
        outb(VGA_CRTC_DATA, *regs);
        regs++;
    }

    // GC
    for (uint8_t i = 0; i < 9; i++)
    {
        outb(VGA_GC_INDEX, i);
        outb(VGA_GC_DATA, *regs);
        regs++;
    }

    // AC
    for (uint8_t i = 0; i < 21; i++)
    {
        inb(VGA_INSTAT_READ);
        outb(VGA_AC_INDEX, i);
        outb(VGA_AC_WRITE, *regs);
        regs++;
    }

    inb(VGA_INSTAT_READ);
    outb(VGA_AC_INDEX, 0x20);
}

void set_plane(uint8_t plane)
{
    uint8_t pmask = 1 << plane;
    outb(VGA_SEQ_INDEX, 0x02);
    outb(VGA_SEQ_DATA, pmask);
}

void set_mode_13h()
{
    // Mode 13h register values
    uint8_t g_320x200x256[] = {
        /* MISC */
        0x63,
        /* SEQ */
        0x03, 0x01, 0x0F, 0x00, 0x0E,
        /* CRTC */
        0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
        0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
        0xFF,
        /* GC */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
        0xFF,
        /* AC */
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x41, 0x00, 0x0F, 0x00, 0x00};

    __asm__ __volatile__(
        "int $0x10"
        :
        : "a"(0x0013));
    write_registers(g_320x200x256);
}

// Function to switch to graphics mode
void switch_to_graphics_mode()
{
    set_mode_13h();
}

// Function to switch back to text mode
void switch_to_text_mode()
{
    // This is a bit trickier in protected mode
    // You might need to implement a full text mode initialization here
    // For now, we'll just reset the video memory
    uint16_t *vm = (uint16_t *)VGA_TEXT_START;
    for (int i = 0; i < VGA_TEXT_WIDTH * VGA_TEXT_HEIGHT; i++)
    {
        vm[i] = 0x0720; // Clear character with black background and white foreground
    }
}