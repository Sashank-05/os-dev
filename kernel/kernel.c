/*#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

void move_cursor(int x, int y) {
    uint16_t pos = y * VGA_WIDTH + x;
    outb(0x3D4, 14);
    outb(0x3D5, (uint8_t)(pos >> 8));
    outb(0x3D4, 15);
    outb(0x3D5, (uint8_t)pos);
}

void clear_screen() {
    volatile uint16_t* video_memory = (volatile uint16_t*)VGA_MEMORY;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        video_memory[i] = (uint16_t)' ' | (uint16_t)(0x0F << 8);
    }
    move_cursor(0, 0);
}

void print(const char* str) {
    volatile uint16_t* video_memory = (volatile uint16_t*)VGA_MEMORY;
    int i = 0;
    while (str[i] != '\0') {
        video_memory[i] = (uint16_t)str[i] | (uint16_t)(0x0F << 8);
        i++;
    }
}

void kernel_main() {
    clear_screen();
    print("Hello from 32-bit C kernel!");
    while(1);
}
*/

void kernel_main()
{
    char *video_memory = (char *)0xb8000;

    for(int i=0; i<8*2; i++)
    {
        *video_memory = 'X';
        *(video_memory+1) = 0x17;
    };
}