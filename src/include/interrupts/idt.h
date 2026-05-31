#pragma once

#include <stddef.h>
#include <stdint.h>

struct interrupt_frame {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
    uint64_t vector;
    uint64_t error_code;
};

extern volatile uint64_t irq_ticks;
extern volatile uint8_t last_scancode;

void interrupts_init(void);
void interrupt_dispatch(struct interrupt_frame* frame);
size_t keyboard_readline(char* buffer, size_t buffer_size);