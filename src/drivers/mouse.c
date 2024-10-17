#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <mem.h>
#include <kernel.h>
#include <boot_print.h>
#include <idt.h>
#include <isr.h>
#include <io.h>

void mouse_callback(registers_t *regs){
    print_string(&cursorx, &cursory, "Mouse input", 0xFFFFFF);
    uint8_t status = inb(0x64);
    while (status & 0x01){
        int8_t mouse_in = inb(0x60);

        if (status & 0x20){

        }
        status = inb(0x64);
    }
    irq_ack(12);
}

uint8_t mouse_timeout(uint8_t signal) {
    unsigned int timeout = 100000;
    while (timeout--) {
        if (signal == 1) {
            if (inb(0x64) & 0x01) {
                return 1;
            }
        } else {
            if (!(inb(0x64) & 0x02)) {
                return 1;
            }
        }
    }
    return 0;  // Timeout occurred
}


uint8_t mouse_read() {
	mouse_timeout(0);
	char t = inb(0x60);
	return t;
}

void mouse_write(uint8_t write) {
	mouse_timeout(1);
	outb(0x64, 0xD4);
	mouse_timeout(1);
	outb(0x60, write);
}

void init_mouse(){
   
    register_interrupt_handler(IRQ12, mouse_callback);
     outb(0x64, 0xA8);
    uint8_t ack = mouse_timeout(1);
    if(ack == 0){
        // boot_print("Mouse not connected\n");
    }


    outb(0x64, 0x20);
    ack = mouse_timeout(0);
    if(ack == 0){
        // boot_print("Mouse not connected\n");
    }

    uint8_t status = (inb(0x60) | 2);
    mouse_timeout(1);
    outb(0x64, 0x60);
    mouse_timeout(1);
    outb(0x60, status);
    mouse_write(0xF6);
	mouse_read();
	mouse_write(0xF4);
	mouse_read();



}