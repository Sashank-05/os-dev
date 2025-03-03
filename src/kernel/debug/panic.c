#include <drivers/framebuffer.h>
#include <stdint.h>
#include <debug/panic.h>
#include <reg.h>
#include <stdarg.h>

void panic(char* string, ...) {
    framebuffer_init(0xFFFFFF);
//    printk(GFX_RED, "\n");
//    printk(GFX_RED, "                             @@                    \n ");
//    printk(GFX_RED, "                           @@@                     \n");
//    printk(GFX_RED, "              @@@         @@@                      \n");
//    printk(GFX_RED, "              @@@        @@                       \n");
//    printk(GFX_RED, "                        @@@                       \n");
//    printk(GFX_RED, "                        @@                        \n");
//    printk(GFX_RED, "                        @@@                       \n");
//    printk(GFX_RED, "                         @@                       \n");
//    printk(GFX_RED, "              @@@         @@                       \n");
//    printk(GFX_RED, "              @@@          @@@                     \n");
//    printk(GFX_RED, "                            @@@                     \n");
//    printk(GFX_RED, "                              @@                    \n");

//    printk(GFX_RED, "KERNEL PANIC: %s\n", string);

    va_list args;
    va_start(args, string);

    struct reg_state_t* registers = NULL;

    // Try to get the register state from variable arguments
    //registers = va_arg(args, struct reg_state_t*);

    if (registers == NULL) {
//        printk(GFX_RED, "Occurred during kernel initialization\nNo trace available\n");
    }
    else {
//        printk(GFX_RED, "Register dump:\n");
//        printk(GFX_RED, "RAX: 0x%x\n", registers->rax);
//        printk(GFX_RED, "RBX: 0x%x\n", registers->rbx);
//        printk(GFX_RED, "RCX: 0x%x\n", registers->rcx);
//        printk(GFX_RED, "RDX: 0x%x\n", registers->rdx);
//        printk(GFX_RED, "RSI: 0x%x\n", registers->rsi);
//        printk(GFX_RED, "RDI: 0x%x\n", registers->rdi);
//        printk(GFX_RED, "RBP: 0x%x\n", registers->rbp);
//        printk(GFX_RED, "RSP: 0x%x\n", registers->rsp);
//        printk(GFX_RED, "R8:  0x%x\n", registers->r8);
//        printk(GFX_RED, "R9:  0x%x\n", registers->r9);
//        printk(GFX_RED, "R10: 0x%x\n", registers->r10);
//        printk(GFX_RED, "R11: 0x%x\n", registers->r11);
//        printk(GFX_RED, "R12: 0x%x\n", registers->r12);
//        printk(GFX_RED, "R13: 0x%x\n", registers->r13);
//        printk(GFX_RED, "R14: 0x%x\n", registers->r14);
//        printk(GFX_RED, "R15: 0x%x\n", registers->r15);
//        printk(GFX_RED, "RIP: 0x%x\n", registers->rip);
    }

//    printk(GFX_RED, "System halted\n");

    va_end(args);
    while (1);
}
