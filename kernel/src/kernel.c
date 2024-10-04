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
    switch_to_graphics_mode();
    
    graphics_clear(0);
    

    
    // Draw a simple pattern
    for (int i = 0; i < 16; i++) {
        vga13h_draw_rect(20 + i * 15, 20 + i * 10, 10, 10, i + 1);
    }
    
    vga13h_draw_line(0, 0, 319, 199, 15);

    print_string(10, 10, "Hello from VGA!!", 26);


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
