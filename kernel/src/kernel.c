#include <drivers/screen.h>
#include <drivers/io.h>
#include <cpu/isr.h>
#include <cpu/timer.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <util.h>
#include <drivers/vga.h>

void sleep(int seconds)
{
    int i = 0;
    while (i < seconds * 100000000)
    {
        asm volatile("nop");
        i++;
    }
}
void memcpy(uint8_t *source, uint8_t *dest, uint32_t nbytes)
{
    int i;
    for (i = 0; i < nbytes; i++)
    {
        *(dest + i) = *(source + i);
    }
}

void memset(void *dest, int value, unsigned int count)
{
    unsigned char *ptr = (unsigned char *)dest;
    while (count--)
    {
        *ptr++ = (unsigned char)value;
    }
}



int main()
{
    int y;
    isr_install();
    asm volatile("sti");
    init_keyboard();
    clear();
    showboot();
    init_mouse();    
    graphics_clear(0);
    

    
    // Draw a simple pattern
    for (int i = 0; i < 256; i++) {
        vga13h_draw_rect(10 + i * 5, 10 + i * 3, 2, 2, i + 1);
    }
    
    //vga13h_draw_line(0, 0, 320, 199, 15);

    // make a rectangle of 256 * 256 pixels of all colors
    for (int i = 32; i < 64; i++) {
         for (int j = 32; j < 64; j++) {
                vga13h_putpixel(5+i, 5+j, i);
            }
    }
    

   // for(y = 0; y < 200; y++)
     //   memset((char *)0xA0000 + (y*320+80), y, 160);

    print_string(10, 10, "Hello from VGA!!", 10);
    return 0;

}

void kernel_main()
{
    main();

    while (1)
    {
        asm volatile("hlt");
    }
}
