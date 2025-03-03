#include <stdint.h>


void disable_interrupts(void){
    __asm__ volatile("cli");
}

void enable_interrupts(void){
    __asm__ volatile("sti");
}

