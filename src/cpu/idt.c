#include <idt.h>
#include <stdint.h>


#define IDT_ENTRIES 256

idt_gate_t idt[IDT_ENTRIES];

idt_register_t idt_reg;

// Update to use 64-bit handler address
void set_idt_gate(int n, uint64_t handler) {
    idt[n].low_offset = (uint16_t)(handler & 0xFFFF);
    idt[n].sel = KERNEL_CS;
    idt[n].ist = 0;     // You can set this to a non-zero value if using IST
    idt[n].flags = 0x8E;
    idt[n].mid_offset = (uint16_t)((handler >> 16) & 0xFFFF);
    idt[n].high_offset = (uint32_t)((handler >> 32) & 0xFFFFFFFF);
    idt[n].reserved = 0;
}

void load_idt() {
    idt_reg.base = (uint64_t)&idt;
    idt_reg.limit = IDT_ENTRIES * sizeof(idt_gate_t) - 1;
    asm volatile("lidt (%0)" : : "r" (&idt_reg));
}