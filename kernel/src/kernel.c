#include <drivers/screen.h>
#include <drivers/io.h>
#include <cpu/isr.h>
#include <cpu/timer.h>

int main(){
    clear();
    showboot();
    boot_print_clear();
    boot_print("[+] Kernel started");
    isr_install();
    asm volatile("sti");
    boot_print("[+] Interrupts enabled!! lesgoo");
    return 0;
}

void kernel_main(){
    main();
}