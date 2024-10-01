#include <drivers/screen.h>
#include <drivers/io.h>
#include <cpu/isr.h>
#include <cpu/timer.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <cpu/pic.h>
#include <cpu/cpuid.h>
#include <util.h>

void sleep(int seconds)
{
    int i = 0;
    while (i < seconds * 100000000)
    {
        asm volatile("nop");
        i++;
    }
}

int main()
{
    init_pic();
    isr_install();
    asm volatile("sti");
    init_keyboard();
    clear();

    showboot();


    init_mouse();

    return 0;
}

void kernel_main()
{
    main();
}
