#include <stdbool.h>
#include <drivers/keyboard.h>
#include <drivers/io.h>
#include <cpu/isr.h>
#include <drivers/screen.h>
#include <util.h>

void mouse_callback(registers_t *regs){
    boot_print("Mouse interrupt");
    uint8_t status = port_byte_in(0x64);
    while (status & 0x01){
        int8_t mouse_in = port_byte_in(0x60);
        boot_print("Mouse input");
        if (status & 0x20){
            boot_print("Received mouse packet");
        }
        status = port_byte_in(0x64);
    }
    irq_ack(12);
}

uint8_t mouse_timeout(uint8_t signal) {
    unsigned int timeout = 100000;
    while (timeout--) {
        if (signal == 1) {
            if (port_byte_in(0x64) & 0x01) {
                return 1;
            }
        } else {
            if (!(port_byte_in(0x64) & 0x02)) {
                return 1;
            }
        }
    }
    return 0;  // Timeout occurred
}


uint8_t mouse_read() {
	mouse_timeout(0);
	char t = port_byte_in(0x60);
	return t;
}

void mouse_write(uint8_t write) {
	mouse_timeout(1);
	port_byte_out(0x64, 0xD4);
	mouse_timeout(1);
	port_byte_out(0x60, write);
}

void init_mouse(){
    // check if mouse is connected
    /*
    All output to port 0x60 or 0x64 must be preceded by waiting for bit 1 (value=2) of port 0x64 to become clear. Similarly, bytes cannot be read from port 0x60 until bit 0 (value=1) of port 0x64 is set.
    */
    // set mouse status on 0x64
   
    register_interrupt_handler(IRQ12, mouse_callback);
     port_byte_out(0x64, 0xA8);
    uint8_t ack = mouse_timeout(1);
    if(ack == 0){
        boot_print("Mouse not connected\n");
    }


    port_byte_out(0x64, 0x20);
    ack = mouse_timeout(0);
    if(ack == 0){
        boot_print("Mouse not connected\n");
    }

    uint8_t status = (port_byte_in(0x60) | 2);
    mouse_timeout(1);
    port_byte_out(0x64, 0x60);
    mouse_timeout(1);
    port_byte_out(0x60, status);
    mouse_write(0xF6);
	mouse_read();
	mouse_write(0xF4);
	mouse_read();



}
