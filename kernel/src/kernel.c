#include <drivers/screen.h>
#include <drivers/io.h>
#include <cpu/isr.h>
#include <cpu/timer.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <cpu/pic.h>

int main(){
    clear();
    showboot();
    
    
    boot_print_clear();
    boot_print("[+] Kernel started");
    init_pic();
    boot_print("[+] PIC initialized");
    isr_install();
    asm volatile("sti");
    boot_print("[+] Interrupts enabled");
    init_keyboard();
    boot_print("[+] Keyboard initialized");
    init_mouse();
    boot_print("[+] Mouse initialized");

    return 0;
}

void kernel_main(){
    main();
}