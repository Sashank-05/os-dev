#include <stdint.h>
#include <stddef.h>

// USB Controller I/O ports
#define USB_BASE 0x20

// Function to write a byte to a USB port
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Function to read a byte from a USB port
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Initialize the USB controller
void usb_init() {
    // Example initialization sequence
    outb(USB_BASE, 0x01); // Reset the controller
    outb(USB_BASE + 1, 0x00); // Clear interrupts
}

// Read data from the USB controller
uint8_t usb_read() {
    return inb(USB_BASE + 2); // Read data from the data port
}

// Write data to the USB controller
void usb_write(uint8_t data) {
    outb(USB_BASE + 2, data); // Write data to the data port
}

