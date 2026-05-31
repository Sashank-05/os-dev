#include <interrupts/idt.h>

#include <debug/panic.h>
#include <debug/debug.h>
#include <drivers/framebuffer.h>
#include <drivers/keyboard.h>
#include <io.h>
#include <libk/stdio.h>
#include <syscall/syscall.h>
#include <stdint.h>
#include <stddef.h>

extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

extern void syscall_stub(void);

volatile uint64_t irq_ticks = 0;
volatile uint8_t last_scancode = 0;

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

static struct idt_entry idt[256] __attribute__((aligned(16)));

static inline void idt_set_gate(uint8_t vector, void (*handler)(void), uint8_t type_attr) {
    uint64_t address = (uint64_t)handler;

    idt[vector].offset_low = address & 0xFFFF;
    idt[vector].selector = 0x08;
    idt[vector].ist = 0;
    idt[vector].type_attr = type_attr;
    idt[vector].offset_mid = (address >> 16) & 0xFFFF;
    idt[vector].offset_high = (address >> 32) & 0xFFFFFFFF;
    idt[vector].zero = 0;
}

static inline void lidt(const struct idt_ptr* ptr) {
    __asm__ volatile ("lidt %0" : : "m"(*ptr));
}

static inline void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(0xA0, 0x20);
    }
    outb(0x20, 0x20);
}

static void pic_remap(void) {
    uint8_t master_mask = inb(0x21);
    uint8_t slave_mask = inb(0xA1);

    outb(0x20, 0x11);
    io_wait();
    outb(0xA0, 0x11);
    io_wait();
    outb(0x21, 0x20);
    io_wait();
    outb(0xA1, 0x28);
    io_wait();
    outb(0x21, 0x04);
    io_wait();
    outb(0xA1, 0x02);
    io_wait();
    outb(0x21, 0x01);
    io_wait();
    outb(0xA1, 0x01);
    io_wait();

    outb(0x21, master_mask & 0xFC);
    outb(0xA1, slave_mask | 0xFF);
}

static void pit_init(uint32_t hz) {
    uint32_t divisor = 1193180 / hz;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
}

static const char* exception_name(uint64_t vector) {
    switch (vector) {
    case 0: return "Divide Error";
    case 1: return "Debug";
    case 2: return "Non-maskable Interrupt";
    case 3: return "Breakpoint";
    case 4: return "Overflow";
    case 5: return "Bound Range Exceeded";
    case 6: return "Invalid Opcode";
    case 7: return "Device Not Available";
    case 8: return "Double Fault";
    case 10: return "Invalid TSS";
    case 11: return "Segment Not Present";
    case 12: return "Stack-Segment Fault";
    case 13: return "General Protection Fault";
    case 14: return "Page Fault";
    case 17: return "Alignment Check";
    case 18: return "Machine Check";
    case 19: return "SIMD Floating-Point";
    case 20: return "Virtualization";
    case 21: return "Control Protection";
    case 29: return "VMM Communication";
    case 30: return "Security";
    default: return "Unknown Exception";
    }
}

void interrupt_dispatch(struct interrupt_frame* frame) {
    if (frame->vector == 32) {
        irq_ticks++;
        pic_send_eoi(0);
        return;
    }

    if (frame->vector == 33) {
        last_scancode = inb(0x60);
        keyboard_handle_scancode(last_scancode);
        pic_send_eoi(1);
        return;
    }

    if (frame->vector >= 32 && frame->vector <= 47) {
        pic_send_eoi((uint8_t)(frame->vector - 32));
        return;
    }

    if (frame->vector == 0x80) {
        syscall_handle(frame);
        return;
    }

        if (frame->vector == 14) {
          uint64_t cr2;
          __asm__ volatile ("mov %%cr2, %0" : "=r"(cr2));
          debug("Page fault %s error=0x%llx cr2=0x%llx\n", exception_name(frame->vector), frame->error_code, cr2);
          debug("R15=0x%llx R14=0x%llx R13=0x%llx R12=0x%llx R11=0x%llx R10=0x%llx R9=0x%llx R8=0x%llx\n",
              frame->r15, frame->r14, frame->r13, frame->r12, frame->r11, frame->r10, frame->r9, frame->r8);
          debug("RBP=0x%llx RDI=0x%llx RSI=0x%llx RDX=0x%llx RCX=0x%llx RBX=0x%llx RAX=0x%llx\n",
              frame->rbp, frame->rdi, frame->rsi, frame->rdx, frame->rcx, frame->rbx, frame->rax);
          debug("vector=%llu error=0x%llx\n", frame->vector, frame->error_code);
          panic("Page fault %s error=0x%llx cr2=0x%llx\n", exception_name(frame->vector), frame->error_code, cr2);
        }
        /* Print saved instruction pointer and segment registers (if present) */
        {
            uint64_t *raw = (uint64_t*)frame;
            uint64_t saved_rip = raw[17];
            uint64_t saved_cs = raw[18];
            uint64_t saved_rflags = raw[19];
            uint64_t saved_rsp = raw[20];
            uint64_t saved_ss = raw[21];
            debug("Saved RIP=0x%llx CS=0x%llx RFLAGS=0x%llx RSP=0x%llx SS=0x%llx\n",
                saved_rip, saved_cs, saved_rflags, saved_rsp, saved_ss);
        }

        debug("CPU exception %s (%llu) error=0x%llx\n", exception_name(frame->vector), frame->vector, frame->error_code);
        debug("R15=0x%llx R14=0x%llx R13=0x%llx R12=0x%llx R11=0x%llx R10=0x%llx R9=0x%llx R8=0x%llx\n",
            frame->r15, frame->r14, frame->r13, frame->r12, frame->r11, frame->r10, frame->r9, frame->r8);
        debug("RBP=0x%llx RDI=0x%llx RSI=0x%llx RDX=0x%llx RCX=0x%llx RBX=0x%llx RAX=0x%llx\n",
            frame->rbp, frame->rdi, frame->rsi, frame->rdx, frame->rcx, frame->rbx, frame->rax);
        debug("vector=%llu error=0x%llx\n", frame->vector, frame->error_code);
        panic("CPU exception %s (%llu) error=0x%llx\n", exception_name(frame->vector), frame->vector, frame->error_code);
}

void interrupts_init(void) {
    for (size_t i = 0; i < 256; i++) {
        idt[i] = (struct idt_entry){0};
    }

    idt_set_gate(0, isr0, 0x8E);
    idt_set_gate(1, isr1, 0x8E);
    idt_set_gate(2, isr2, 0x8E);
    idt_set_gate(3, isr3, 0x8E);
    idt_set_gate(4, isr4, 0x8E);
    idt_set_gate(5, isr5, 0x8E);
    idt_set_gate(6, isr6, 0x8E);
    idt_set_gate(7, isr7, 0x8E);
    idt_set_gate(8, isr8, 0x8E);
    idt_set_gate(9, isr9, 0x8E);
    idt_set_gate(10, isr10, 0x8E);
    idt_set_gate(11, isr11, 0x8E);
    idt_set_gate(12, isr12, 0x8E);
    idt_set_gate(13, isr13, 0x8E);
    idt_set_gate(14, isr14, 0x8E);
    idt_set_gate(15, isr15, 0x8E);
    idt_set_gate(16, isr16, 0x8E);
    idt_set_gate(17, isr17, 0x8E);
    idt_set_gate(18, isr18, 0x8E);
    idt_set_gate(19, isr19, 0x8E);
    idt_set_gate(20, isr20, 0x8E);
    idt_set_gate(21, isr21, 0x8E);
    idt_set_gate(22, isr22, 0x8E);
    idt_set_gate(23, isr23, 0x8E);
    idt_set_gate(24, isr24, 0x8E);
    idt_set_gate(25, isr25, 0x8E);
    idt_set_gate(26, isr26, 0x8E);
    idt_set_gate(27, isr27, 0x8E);
    idt_set_gate(28, isr28, 0x8E);
    idt_set_gate(29, isr29, 0x8E);
    idt_set_gate(30, isr30, 0x8E);
    idt_set_gate(31, isr31, 0x8E);

    idt_set_gate(32, irq0, 0x8E);
    idt_set_gate(33, irq1, 0x8E);
    idt_set_gate(34, irq2, 0x8E);
    idt_set_gate(35, irq3, 0x8E);
    idt_set_gate(36, irq4, 0x8E);
    idt_set_gate(37, irq5, 0x8E);
    idt_set_gate(38, irq6, 0x8E);
    idt_set_gate(39, irq7, 0x8E);
    idt_set_gate(40, irq8, 0x8E);
    idt_set_gate(41, irq9, 0x8E);
    idt_set_gate(42, irq10, 0x8E);
    idt_set_gate(43, irq11, 0x8E);
    idt_set_gate(44, irq12, 0x8E);
    idt_set_gate(45, irq13, 0x8E);
    idt_set_gate(46, irq14, 0x8E);
    idt_set_gate(47, irq15, 0x8E);

    idt_set_gate(0x80, syscall_stub, 0xEE);

    struct idt_ptr ptr = {
        .limit = sizeof(idt) - 1,
        .base = (uint64_t)&idt[0]
    };

    pic_remap();
    pit_init(100);
    lidt(&ptr);
}