#include <drivers/screen.h>
#include <drivers/io.h>
#include <cpu/isr.h>
#include <cpu/timer.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <cpu/pic.h>
#include <cpu/cpuid.h>
#include <util.h>
#include <drivers/vga.h>


typedef struct __attribute__((packed))
{
    unsigned short di, si, bp, sp, bx, dx, cx, ax;
    unsigned short gs, fs, es, ds, eflags;
} regs16_t;

// tell compiler our int32 function is external
extern void int32(unsigned char intnum, regs16_t *regs);

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

void set_palette(uint8_t index, uint8_t red, uint8_t green, uint8_t blue) {
    port_byte_out(0x3C8, index); // Palette index
    port_byte_out(0x3C9, red);   // Red component
    port_byte_out(0x3C9, green); // Green component
    port_byte_out(0x3C9, blue);  // Blue component
    
}

void initialize_palette() {
    for (int i = 0; i < 256; i++) {
        set_palette(i, i, i, i); // Simple grayscale palette
    }
}

int main()
{
    int y;
    regs16_t regs;
    init_pic();
    isr_install();
    asm volatile("sti");
    init_keyboard();
    clear();

    showboot();

    init_mouse();
    //switch_to_graphics_mode();
    //initialize_palette();
    
    graphics_clear(0);
    

    
    // Draw a simple pattern
    for (int i = 0; i < 256; i++) {
        vga13h_draw_rect(10 + i * 5, 10 + i * 3, 2, 2, i + 1);
    }
    
    //vga13h_draw_line(0, 0, 319, 199, 15);

    

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
