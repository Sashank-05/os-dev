#ifndef GDT_H
#define GDT_H

#include <stdint.h>

// GDT Entry (8 bytes)
struct __attribute__((packed)) gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
};

// GDT Pointer
struct __attribute__((packed)) gdt_ptr {
    uint16_t limit;
    uint64_t base;
};

// 64-bit Task State Segment (TSS)
struct __attribute__((packed)) tss_entry {
    uint32_t reserved1;
    uint64_t rsp0;  // Kernel stack for Ring 0
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved2;
    uint64_t ist1;  // Interrupt Stack Table (IST) (for faults)
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved3;
    uint16_t reserved4;
    uint16_t iomap_base;
};

// Functions
void gdt_init();
void tss_init(uint64_t rsp0);

#define GDT_KERNEL_CODE 0x08
#define GDT_KERNEL_DATA 0x10
#define GDT_USER_DATA   0x18
#define GDT_USER_CODE   0x20
#define GDT_TSS         0x28

#endif
