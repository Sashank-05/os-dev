#ifndef _STDINFO_H
#define _STDINFO_H
#include <stdint.h>
#include <drivers/acpi.h>
#include "../limine.h"

struct OSinfo {
    long boot_time;
    struct limine_hhdm_response* hhdm_response;
    struct limine_memmap_response* memmap;
    struct limine_framebuffer_response* framebuffer;
    struct limine_rsdp_response* rsdp_response;
    struct limine_smp_response* smp;
    struct limine_module_response* initrd;
    struct limine_kernel_address_response kernel_addr;

    uint64_t hhdm;

    uintptr_t kheap;
    
    RSDT* rsdt;
    RSDP* rsdp;

    unsigned long long total_memory;
    unsigned long free_memory;

    unsigned long freeswap;
    unsigned long totalswap;

    unsigned long handles;

    unsigned long long cr3;


};

enum LOG_LEVEL {
    INFO,
    WARN,
    ERROR,
    DEBUG
};

#endif
