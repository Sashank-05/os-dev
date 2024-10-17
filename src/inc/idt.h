#pragma once

#include <stdint.h>

#define KERNEL_CS 0x08


typedef struct {
    uint16_t low_offset;
    uint16_t sel;
    uint8_t ist;        // New: Interrupt Stack Table
    uint8_t flags;
    uint16_t mid_offset;
    uint32_t high_offset;
    uint32_t reserved;
} __attribute__((packed)) idt_gate_t;

typedef struct {
    uint16_t limit;
    uint64_t base;      // Changed to 64-bit
} __attribute__((packed)) idt_register_t;

#define IDT_ENTRIES 256

void set_idt_gate(int n, uint64_t handler);

void load_idt();