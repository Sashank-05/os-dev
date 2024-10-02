#include <drivers/screen.h>
#include <drivers/io.h>
#include <cpu/isr.h>
#include <cpu/timer.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <cpu/pic.h>
#include <cpu/cpuid.h>
#include <util.h>

struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    // ... add more fields if needed
};

uint32_t get_eflags() {
    uint32_t eflags;
    asm volatile (
        "pushf\n\t"        // Push EFLAGS onto the stack
        "pop %0\n\t"       // Pop it into eflags
        : "=r" (eflags)    // Output operand
    );
    return eflags;
}

uint16_t get_cs() {
    uint16_t cs;
    asm volatile (
        "mov %%cs, %0\n\t" // Move CS into cs
        : "=r" (cs)        // Output operand
    );
    return cs;
}

void check_cpu_mode() {
    uint32_t eflags = get_eflags();
    uint16_t cs = get_cs();

    if (eflags & (1 << 1)) { // Check PE bit (bit 1)
        boot_print("[+] Protected mode!!");
    } else {
        // We're in real mode
        boot_print("[!] Real mode :(");
    }

    // Optionally, check the CS value (if you have specific descriptors)
}


void sleep(int seconds) {
    int i = 0;
    while (i < seconds * 100000000)
    {
        asm volatile("nop");
        i++;
    }
}

int main() {
    clear();
    showboot();
    check_cpu_mode();
    init_pic();
    isr_install();
    boot_print("[+] Installed ISRs");
    asm volatile("sti");
    init_keyboard();
    boot_print("[+] Enabled interrupts");
    
    boot_print("[+] Initialized keyboard");
    boot_print("Hello, World!");
    //init_mouse();
    return 0;
}

void kernel_main(uint32_t multiboot_magic, struct multiboot_info* mboot_ptr) {
    if (multiboot_magic != 0x2BADB002 || multiboot_magic != 0x1BADB002) {
        // If not loaded by GRUB, print error
        const char* error = "Error: Not loaded by GRUB!";
        volatile char* video = (volatile char*)0xB8000;
        for (const char* ptr = error; *ptr != '\0'; ptr++) {
            *video++ = *ptr;
            *video++ = 0x4F; // White on red
        }
        putss("Error: Not loaded by GRUB!\n");
        char s[3];
        int_to_string(multiboot_magic, s);
        putss(s);
       sleep(5);
    }
    // Call your original main function
    main();
    // If main returns, halt the CPU
    for(;;) {
        asm volatile("hlt");
    }
}