#pragma once

#include <stdint.h>
#include <interrupts/idt.h>

enum syscall_number {
    SYS_write = 0,
    SYS_exit = 1,
    SYS_ticks = 2,
    SYS_net_send = 3,
    SYS_readline = 4,
    SYS_listdir = 5,
    SYS_readfile = 6,
    SYS_exec = 7,
    SYS_sleep = 8,
    SYS_get_framebuffer = 9,
    SYS_filesize = 10,
    SYS_keyevent = 11,
};

void syscall_init(void);
void syscall_handle(struct interrupt_frame* frame);
