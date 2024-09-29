#include <drivers/screen.h>
#include <drivers/io.h>
#include <cpu/isr.h>
#include <cpu/timer.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <cpu/pic.h>


void sleep(int seconds){
    int i = 0;
    while(i < seconds * 100000000){
        asm volatile("nop");
        i++;
    }
}

int main(){
    //switch_to_text_mode();
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

    sleep(3);

    //clear();

    //switch_to_graphics_mode();
    
    //graphics_clear(0);

    //graphics_showboot();

    //graphics_clear(0);

    //switch_to_text_mode();
    //clear();
    //putss("Hello, World!");
 

    return 0;
}



void kernel_main(){
    main();
}