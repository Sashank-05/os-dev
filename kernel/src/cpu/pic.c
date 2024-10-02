#include <drivers/io.h>
#include <cpu/pic.h>

// PIC ports and constants
#define PIC1 0x20
#define PIC2 0xA0
#define PIC1_DATA 0x21
#define PIC2_DATA 0xA1
#define PIC_EOI 0x20

void pic_remap() {
    // ICW1 - start initialization
    port_byte_out(PIC1, 0x11);
    port_byte_out(PIC2, 0x11);

    // ICW2 - remap offset address for master and slave PIC
    port_byte_out(PIC1_DATA, 0x20);  // Master PIC starts at 0x20
    port_byte_out(PIC2_DATA, 0x28);  // Slave PIC starts at 0x28

    // ICW3 - tell Master PIC there is a Slave PIC at IRQ2
    port_byte_out(PIC1_DATA, 0x04);
    port_byte_out(PIC2_DATA, 0x02);

    // ICW4 - set mode
    port_byte_out(PIC1_DATA, 0x01);
    port_byte_out(PIC2_DATA, 0x01);

    // Mask all interrupts initially
    port_byte_out(PIC1_DATA, 0xFF);
    port_byte_out(PIC2_DATA, 0xFF);
}

void irq_unmask() {
    uint8_t master_mask = port_byte_in(PIC1_DATA);
    uint8_t slave_mask = port_byte_in(PIC2_DATA);

    // Unmask IRQ1 (keyboard) and IRQ12 (mouse)
    master_mask &= ~(1 << 1);  // Unmask IRQ1 (keyboard)
    slave_mask &= ~(1 << 4);   // Unmask IRQ12 (mouse)

    // Write the new masks back to the PIC
    port_byte_out(PIC1_DATA, master_mask);
    port_byte_out(PIC2_DATA, slave_mask);
}

void init_pic() {
    pic_remap();   // Remap the PIC
    irq_unmask();  // Unmask keyboard and mouse IRQs
}
