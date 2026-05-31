#include <common/init.h>
#include <limine.h>
#include <drivers/serial.h>
#include <debug/debug.h>
#include <drivers/framebuffer.h>
#include <memory/pmm.h>
#include <sync/spinlock.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <common/stdinfo.h>
#include <libk/stdio.h>
#include <debug/panic.h>
#include <memory/kheap.h>
#include <common/gdt.h>
#include <memory/paging.h>
#include <reg.h>
#include <drivers/acpi.h>
#include <fs/vfs.h>
#include <net/net.h>
#include <syscall/syscall.h>
#include <process/process.h>
#include <libk/stdio.h>
#include <interrupts/idt.h>
#include <asm/asmcalls.h>


struct OSinfo osinfo;

unsigned long long total_mem(struct limine_memmap_response* memmap) {
    const uint64_t valid_types[] = { LIMINE_MEMMAP_USABLE, LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE, LIMINE_MEMMAP_KERNEL_AND_MODULES, LIMINE_MEMMAP_FRAMEBUFFER, LIMINE_MEMMAP_ACPI_RECLAIMABLE};
    unsigned long long total = 0;
    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap->entries[i];
        for (size_t j = 0; j < sizeof(valid_types) / sizeof(valid_types[0]); j++) {
            if (entry->type == valid_types[j]) {
                total += entry->length;
                break;
            }
        }
    }
    return total;

}

void init_kernel(void) {
    osinfo.boot_time = boot_time_request.response->boot_time;
    osinfo.memmap = memmap_request.response;
    osinfo.hhdm_response = hhdm_request.response;
    osinfo.hhdm = osinfo.hhdm_response->offset;
    osinfo.rsdp_response = rsdp_request.response;
    osinfo.rsdp = (RSDP*)osinfo.rsdp_response->address;
    osinfo.kernel_addr = *(kernel_address_request.response);
    osinfo.initrd = initrd_request.response;
    osinfo.rsdp = rsdp_request.response->address;
    osinfo.smp = smp_request.response;
    osinfo.framebuffer = framebuffer_request.response;
    osinfo.totalswap = 0;
    osinfo.free_memory = 0;
    osinfo.total_memory = total_mem(osinfo.memmap);
    osinfo.freeswap = 0;
    osinfo.handles = 0;
}

void print_system_info(void) {
    printk(GFX_WHITE, "=== System Information ===\n");
    debug(ANSI_WHITE "=== System Information ===\n");

    printk(GFX_WHITE, "Compiled on: %s at %s\n", __DATE__, __TIME__);
    debug(ANSI_WHITE "Compiled on: %s at %s\n", __DATE__, __TIME__);

    printk(GFX_WHITE, "Boot time: %llu\n", osinfo.boot_time);
    debug(ANSI_WHITE "Boot time: %llu\n", osinfo.boot_time);

    if (osinfo.smp) {
        printk(GFX_WHITE, "CPU Cores: %u\n", osinfo.smp->cpu_count);
        debug(ANSI_WHITE "CPU Cores: %u\n", osinfo.smp->cpu_count);
    }

    if (osinfo.framebuffer) {
        struct limine_framebuffer* fb = osinfo.framebuffer->framebuffers[0];
        printk(GFX_WHITE, "Framebuffer: %ux%u, %u BPP\n",
            fb->width, fb->height, fb->bpp);
        debug(ANSI_WHITE "Framebuffer: %ux%u, %u BPP\n",
            fb->width, fb->height, fb->bpp);
    }

    printk(GFX_WHITE, "Total Memory: %llu bytes\n", osinfo.total_memory);
    debug(ANSI_WHITE "Total Memory: %llu bytes\n", osinfo.total_memory);

    printk(GFX_WHITE, "Free Memory: %llu bytes\n", osinfo.free_memory);
    debug(ANSI_WHITE "Free Memory: %llu bytes\n", osinfo.free_memory);

    printk(GFX_WHITE, "=======================\n");
    debug(ANSI_WHITE "=======================\n");
}



void kmain(void){
    disable_interrupts();
    init_kernel();
    framebuffer_init(GFX_BLACK);
    serial_init(COM1);

    print_system_info();

    pmm_init();
    debug("PMM initialized successfully\n");

    kheap_init();
    debug("Kernel heap initialized successfully\n");

    init_paging();
    debug("Paging initialized successfully\n");
    debug("Initializing VFS\n");

    debug("Initializing networking\n");
    debug("Initializing GDT\n");
    debug("Initializing syscall ABI\n");
    gdt_init();
    debug("Initializing interrupts\n");
    debug("Initializing TSS\n");
    tss_init(KERNEL_STACK_PTR);
    debug("GDT/TSS initialized successfully\n");

    vfs_init();
    debug("Enabling interrupts\n");
    net_init();
    syscall_init();
    interrupts_init();

    init_acpi();
    debug("ACPI initialized successfully\n");

    enable_interrupts();

    debug("Bootstrapping userspace init\n");
    process_bootstrap();

    while (1) {
        __asm__ __volatile__("hlt");
    };

}

/*
void kmain(void) {
    init_kernel();

    struct reg_state_t regs;
    framebuffer_init(GFX_BLACK);
    serial_init(COM1);

//    printk(GFX_WHITE, "\nStarting kernel...\n");
    print_system_info();

//    printk(GFX_WHITE, "Initializing PMM...\n");
    pmm_init();
//    printk(GFX_WHITE, "PMM initialized successfully\n");

//    printk(GFX_WHITE, "Memory allocation test:\n");
//    printk(GFX_WHITE, "- Allocating 5 pages...\n");
    void* addr = pmm_alloc_pages(5);
//    printk(GFX_WHITE, "- Pages allocated at address: 0x%llx\n", addr);
//    printk(GFX_WHITE, "- Freeing 5 pages...\n");

//    printk(GFX_WHITE, "- Memory test completed\n");


//    printk(GFX_WHITE, "Kernel heap test:\n");
    kheap_init();
//    printk(GFX_WHITE, "Kernel heap test completed\n");

    // test kmalloc and kfree
//    printk(GFX_WHITE, "Testing kmalloc and kfree\n");


//    printk(GFX_WHITE, "Testing kmalloc and kfree completed\n");

//    printk(GFX_WHITE, "Testing paging\n");
//    printk(GFX_WHITE, "CPU supports 5-level paging: %s\n", is5lvl() ? "yes" : "no");
//    printk(GFX_WHITE, "Before paging: osinfo.memmap = %p\n", osinfo.memmap);
    init_paging();
//    printk(GFX_WHITE, "After paging: osinfo.memmap = %p\n", osinfo.memmap);
//    printk(GFX_PURPLE, "\nCR3: 0x%llx\n", osinfo.cr3);

    switch_page_structures();
//    printk(GFX_WHITE, "paging completed\n");

//    printk(GFX_WHITE, "Initializing ACPI\n");
    init_acpi();

//    printk(GFX_WHITE, "ACPI initialization complete\n");



    while (1) {
        __asm__ __volatile__("hlt");
    };
}

*/
