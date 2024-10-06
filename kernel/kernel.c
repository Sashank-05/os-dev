#include <cpu/idt.h>
#include <cpu/isr.h>
#include <cpu/timer.h>
#include <drivers/screen.h>
#include <drivers/io.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <util.h>
#include <mem.h>


extern int curr_line;

int main() {
    isr_install();
    init_keyboard();
    init_mouse();
    __asm__ __volatile__("sti");

    print_string(5, 5, "Hello from VGA!", 15);

    // draw rectangle of all colors
    for (int i = 32; i < 64; i++) {
        for (int j = 32; j < 64; j++) {
            put_pixel(i, j, i);
        }
    }
    

    return 0;
}

void kernel_main() {
    main();
    while (1) {}
}
