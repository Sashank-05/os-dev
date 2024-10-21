#include <idt.h>
#include <isr.h>
#include <mem.h>
#include <boot_print.h>
#include <stdint.h>
#include <kernel.h>
#include <io.h>
#include <util.h>

extern int curr_line;
isr_t interrupt_handlers[256];

void isr_install()
{
    // Update to use 64-bit function pointers
    set_idt_gate(0, (uint64_t)isr0);
    set_idt_gate(1, (uint64_t)isr1);
    set_idt_gate(2, (uint64_t)isr2);
    set_idt_gate(3, (uint64_t)isr3);
    set_idt_gate(4, (uint64_t)isr4);
    set_idt_gate(5, (uint64_t)isr5);
    set_idt_gate(6, (uint64_t)isr6);
    set_idt_gate(7, (uint64_t)isr7);
    set_idt_gate(8, (uint64_t)isr8);
    set_idt_gate(9, (uint64_t)isr9);
    set_idt_gate(10, (uint64_t)isr10);
    set_idt_gate(11, (uint64_t)isr11);
    set_idt_gate(12, (uint64_t)isr12);
    set_idt_gate(13, (uint64_t)isr13);
    set_idt_gate(14, (uint64_t)isr14);
    set_idt_gate(15, (uint64_t)isr15);
    set_idt_gate(16, (uint64_t)isr16);
    set_idt_gate(17, (uint64_t)isr17);
    set_idt_gate(18, (uint64_t)isr18);
    set_idt_gate(19, (uint64_t)isr19);
    set_idt_gate(20, (uint64_t)isr20);
    set_idt_gate(21, (uint64_t)isr21);
    set_idt_gate(22, (uint64_t)isr22);
    set_idt_gate(23, (uint64_t)isr23);
    set_idt_gate(24, (uint64_t)isr24);
    set_idt_gate(25, (uint64_t)isr25);
    set_idt_gate(26, (uint64_t)isr26);
    set_idt_gate(27, (uint64_t)isr27);
    set_idt_gate(28, (uint64_t)isr28);
    set_idt_gate(29, (uint64_t)isr29);
    set_idt_gate(30, (uint64_t)isr30);
    set_idt_gate(31, (uint64_t)isr31);

    // Remap the PIC
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    // Install the IRQs (update to use 64-bit function pointers)
    set_idt_gate(32, (uint64_t)irq0);
    set_idt_gate(33, (uint64_t)irq1);
    set_idt_gate(34, (uint64_t)irq2);
    set_idt_gate(35, (uint64_t)irq3);
    set_idt_gate(36, (uint64_t)irq4);
    set_idt_gate(37, (uint64_t)irq5);
    set_idt_gate(38, (uint64_t)irq6);
    set_idt_gate(39, (uint64_t)irq7);
    set_idt_gate(40, (uint64_t)irq8);
    set_idt_gate(41, (uint64_t)irq9);
    set_idt_gate(42, (uint64_t)irq10);
    set_idt_gate(43, (uint64_t)irq11);
    set_idt_gate(44, (uint64_t)irq12);
    set_idt_gate(45, (uint64_t)irq13);
    set_idt_gate(46, (uint64_t)irq14);
    set_idt_gate(47, (uint64_t)irq15);

    load_idt(); // Load with ASM
}

// Exception messages remain the same
char *exception_messages[] = {
    "Division By Zero", "Debug", "Non Maskable Interrupt", "Breakpoint",
    "Into Detected Overflow", "Out of Bounds", "Invalid Opcode", "No Coprocessor",
    "Double Fault", "Coprocessor Segment Overrun", "Bad TSS", "Segment Not Present",
    "Stack Fault", "General Protection Fault", "Page Fault", "Unknown Interrupt",
    "Coprocessor Fault", "Alignment Check", "Machine Check", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved"};

void isr_handler(registers_t *r)
{
    clearscreen(0xF000FA);

    // set cursors to 0
    cursorx = 0;
    cursory = 0;

    char int_no_str[21]; // Buffer for interrupt number
    itoa(r->int_no, int_no_str);

    // print a nice message
    print_string(&cursorx, &cursory, "Kernel Error!\n\n", 0xFFFFFF);

    // Print the interrupt number
    print_string(&cursorx, &cursory, "Interrupt Number: ", 0xFFFFFF);
    print_string(&cursorx, &cursory, int_no_str, 0xFFFFFF);
    print_string(&cursorx, &cursory, "\n", 0xFFFFFF);

    // Print error code if applicable
    if (r->int_no < 32)
    {                            // Check if it's an exception
        char error_code_str[21]; // Buffer for error code
        itoa(r->error_code, error_code_str);
        print_string(&cursorx, &cursory, "Error Code: ", 0xFFFFFF);
        print_string(&cursorx, &cursory, error_code_str, 0xFFFFFF);
        print_string(&cursorx, &cursory, "\n", 0xFFFFFF);
    }
    if (r->int_no < 32)
    {
        print_string(&cursorx, &cursory, exception_messages[r->int_no], 0xFF0000);
        print_string(&cursorx, &cursory, "\n", 0xFFFFFF);
    }
    else
    {
        print_string(&cursorx, &cursory, "Unknown interrupt\n", 0xFF0000);
        print_string(&cursorx, &cursory, "\n", 0xFFFFFF);
    }

    // Print register information
    char hex_buffer[20]; // Buffer for hexadecimal conversion

    print_string(&cursorx, &cursory, "Registers:\n", 0xFFFFFF);

    htoa(r->rax, hex_buffer);
    print_string(&cursorx, &cursory, "RAX: 0x", 0xFFFFFF);
    print_string(&cursorx, &cursory, hex_buffer, 0xFFFFFF);
    print_string(&cursorx, &cursory, "\n", 0xFFFFFF);
    htoa(r->rbx, hex_buffer);
    print_string(&cursorx, &cursory, "RBX: 0x", 0xFFFFFF);
    print_string(&cursorx, &cursory, hex_buffer, 0xFFFFFF);
    print_string(&cursorx, &cursory, "\n", 0xFFFFFF);
    htoa(r->rcx, hex_buffer);
    print_string(&cursorx, &cursory, "RCX: 0x", 0xFFFFFF);
    print_string(&cursorx, &cursory, hex_buffer, 0xFFFFFF);
    print_string(&cursorx, &cursory, "\n", 0xFFFFFF);
    htoa(r->rdx, hex_buffer);
    print_string(&cursorx, &cursory, "RDX: 0x", 0xFFFFFF);
    print_string(&cursorx, &cursory, hex_buffer, 0xFFFFFF);
    print_string(&cursorx, &cursory, "\n", 0xFFFFFF);
    htoa(r->rsi, hex_buffer);
    print_string(&cursorx, &cursory, "RSI: 0x", 0xFFFFFF);
    print_string(&cursorx, &cursory, hex_buffer, 0xFFFFFF);
    print_string(&cursorx, &cursory, "\n", 0xFFFFFF);
    htoa(r->rdi, hex_buffer);
    print_string(&cursorx, &cursory, "RDI: 0x", 0xFFFFFF);
    print_string(&cursorx, &cursory, hex_buffer, 0xFFFFFF);
    print_string(&cursorx, &cursory, "\n", 0xFFFFFF);
    htoa(r->rbp, hex_buffer);
    print_string(&cursorx, &cursory, "RBP: 0x", 0xFFFFFF);
    print_string(&cursorx, &cursory, hex_buffer, 0xFFFFFF);
    print_string(&cursorx, &cursory, "\n", 0xFFFFFF);
    htoa(r->rsp, hex_buffer);
    print_string(&cursorx, &cursory, "RSP: 0x", 0xFFFFFF);
    print_string(&cursorx, &cursory, hex_buffer, 0xFFFFFF);
    print_string(&cursorx, &cursory, "\n", 0xFFFFFF);
    htoa(r->r8, hex_buffer);
    print_string(&cursorx, &cursory, "R8: 0x", 0xFFFFFF);
    print_string(&cursorx, &cursory, hex_buffer, 0xFFFFFF);
    print_string(&cursorx, &cursory, "\n", 0xFFFFFF);
    htoa(r->r9, hex_buffer);
    print_string(&cursorx, &cursory, "R9: 0x", 0xFFFFFF);
    print_string(&cursorx, &cursory, hex_buffer, 0xFFFFFF);
    print_string(&cursorx, &cursory, "\n", 0xFFFFFF);
    htoa(r->r10, hex_buffer);
    print_string(&cursorx, &cursory, "R10: 0x", 0xFFFFFF);
    print_string(&cursorx, &cursory, hex_buffer, 0xFFFFFF);
    print_string(&cursorx, &cursory, "\n", 0xFFFFFF);
    htoa(r->r11, hex_buffer);
    print_string(&cursorx, &cursory, "R11: 0x", 0xFFFFFF);
    print_string(&cursorx, &cursory, hex_buffer, 0xFFFFFF);
    print_string(&cursorx, &cursory, "\n", 0xFFFFFF);
    htoa(r->r12, hex_buffer);
    print_string(&cursorx, &cursory, "R12: 0x", 0xFFFFFF);
    print_string(&cursorx, &cursory, hex_buffer, 0xFFFFFF);
    print_string(&cursorx, &cursory, "\n", 0xFFFFFF);
    htoa(r->r13, hex_buffer);
    print_string(&cursorx, &cursory, "R13: 0x", 0xFFFFFF);
    print_string(&cursorx, &cursory, hex_buffer, 0xFFFFFF);
    print_string(&cursorx, &cursory, "\n", 0xFFFFFF);
    htoa(r->r14, hex_buffer);
    print_string(&cursorx, &cursory, "R14: 0x", 0xFFFFFF);
    print_string(&cursorx, &cursory, hex_buffer, 0xFFFFFF);
    print_string(&cursorx, &cursory, "\n", 0xFFFFFF);
    htoa(r->r15, hex_buffer);
    print_string(&cursorx, &cursory, "R15: 0x", 0xFFFFFF);
    print_string(&cursorx, &cursory, hex_buffer, 0xFFFFFF);
    print_string(&cursorx, &cursory, "\n", 0xFFFFFF);
    htoa(r->rip, hex_buffer);
    print_string(&cursorx, &cursory, "RIP: 0x", 0xFFFFFF);
    print_string(&cursorx, &cursory, hex_buffer, 0xFFFFFF);
    print_string(&cursorx, &cursory, "\n", 0xFFFFFF);
    htoa(r->rflags, hex_buffer);
    print_string(&cursorx, &cursory, "RFLAGS: 0x", 0xFFFFFF);
    print_string(&cursorx, &cursory, hex_buffer, 0xFFFFFF);
    print_string(&cursorx, &cursory, "\n", 0xFFFFFF);
    htoa(r->cs, hex_buffer);
    print_string(&cursorx, &cursory, "CS: 0x", 0xFFFFFF);
    print_string(&cursorx, &cursory, hex_buffer, 0xFFFFFF);
    print_string(&cursorx, &cursory, "\n", 0xFFFFFF);
    // htoa(r->ds, hex_buffer); print_string(&cursorx, &cursory, "DS: 0x", 0xFFFFFF); print_string(&cursorx, &cursory, hex_buffer, 0xFFFFFF);print_string(&cursorx, &cursory, "\n", 0xFFFFFF);
    htoa(r->ss, hex_buffer);
    print_string(&cursorx, &cursory, "SS: 0x", 0xFFFFFF);
    print_string(&cursorx, &cursory, hex_buffer, 0xFFFFFF);
    print_string(&cursorx, &cursory, "\n", 0xFFFFFF);

    while (1)
    {
        asm volatile("hlt");
    };
}

void register_interrupt_handler(uint8_t n, isr_t handler)
{
    interrupt_handlers[n] = handler;
}

void irq_ack(int irq_no)
{
    if (irq_no >= 12)
    {
        outb(0xA0, 0x20);
    }
    outb(0x20, 0x20);
}

void irq_handler(registers_t *r)
{
    if (interrupt_handlers[r->int_no] != 0)
    {
        isr_t handler = interrupt_handlers[r->int_no];
        handler(r);
    }

    // EOI
    if (r->int_no >= 40)
    {
        outb(0xA0, 0x20); /* follower */
    }
    outb(0x20, 0x20); /* leader */
}