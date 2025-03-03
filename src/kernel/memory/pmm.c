#include <memory/pmm.h>
#include <limine.h>
#include <debug/debug.h>
#include <drivers/framebuffer.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <common/stdinfo.h>
#include <sync/spinlock.h>


#define PAGE_SIZE 4096
#define MIN_MEMORY (4 * 1024 * 1024)  // 4MB minimum memory requirement

static uint8_t* bitmap = NULL;
static uint64_t total_pages = 0;
static uint64_t last_allocated = 0;
static spinlock_t pmm_lock = SPINLOCK_INIT;

extern struct OSinfo osinfo;

static const char* get_memory_type_str(uint64_t type) {
    switch (type) {
    case LIMINE_MEMMAP_USABLE:
        return "FREE";
    case LIMINE_MEMMAP_RESERVED:
        return "RESV";
    case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
        return "ACPI";
    case LIMINE_MEMMAP_ACPI_NVS:
        return "NVRAM";
    case LIMINE_MEMMAP_BAD_MEMORY:
        return "BAD";
    case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
        return "BOOT";
    case LIMINE_MEMMAP_KERNEL_AND_MODULES:
        return "KERN";
    case LIMINE_MEMMAP_FRAMEBUFFER:
        return "FB";
    default:
        return "????";
    }
}

static void print_memory_map_header(void) {
    debug(ANSI_CYAN "┌───────────────────── Memory Map ────────────────────────┐\n");
    //    printk(GFX_CYAN, "┌───────────────────── Memory Map ────────────────────────┐\n");

    debug(ANSI_CYAN "├──────┬────────────────────┬────────────────────┬────────┤\n");
    //    printk(GFX_CYAN, "├──────┬────────────────────┬────────────────────┬────────┤\n");

    debug(ANSI_WHITE "│ ID   │      Base          │     Length         │  Type  │\n");
    //    printk(GFX_WHITE, "│ ID   │      Base          │     Length         │  Type  │\n");

    debug(ANSI_CYAN "├──────┼────────────────────┼────────────────────┼────────┤\n");
    //    printk(GFX_CYAN, "├──────┼────────────────────┼────────────────────┼────────┤\n");
}

static void print_memory_entry(size_t id, struct limine_memmap_entry* entry) {
    debug(ANSI_GREEN "│ %-4zu │ 0x%012llx     │ 0x%012llx     │ %-6s │\n",
        id, entry->base, entry->length, get_memory_type_str(entry->type));
    // printk(GFX_GREEN, "│ %-4zu │ 0x%012llx     │ 0x%012llx     │ %-6s │\n",
      //   id, entry->base, entry->length, get_memory_type_str(entry->type));
}

static void print_memory_map_footer(void) {
    debug(ANSI_CYAN "└──────┴────────────────────┴────────────────────┴────────┘\n");
    //    printk(GFX_CYAN, "└──────┴────────────────────┴────────────────────┴────────┘\n");
}

static void set_bit(uint64_t page) {
    bitmap[page / 8] |= (1 << (page % 8));
}

static void clear_bit(uint64_t page) {
    bitmap[page / 8] &= ~(1 << (page % 8));
}

static bool test_bit(uint64_t page) {
    return (bitmap[page / 8] & (1 << (page % 8))) != 0;
}

void pmm_init(void) {
    struct limine_memmap_response* memmap = osinfo.memmap;
    struct limine_hhdm_response* hhdm = osinfo.hhdm_response;
    uint64_t total_memory = 0;
    uint64_t bitmap_pages = 0;

    if (!memmap || !hhdm || !memmap->entry_count) {
        debug(ANSI_RED "Critical memory map information missing!");
        //panic("No valid memory map available");
    }

    // Calculate total physical memory and find largest usable region
    uint64_t max_usable_base = 0;
    uint64_t max_usable_size = 0;

    print_memory_map_header();

    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap->entries[i];
        print_memory_entry(i, entry);

        if (entry->type == LIMINE_MEMMAP_USABLE || entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
            total_memory += entry->length;
            if (entry->length > max_usable_size) {
                max_usable_size = entry->length;
                max_usable_base = entry->base;
            }
        }
    }

    print_memory_map_footer();

    if (total_memory < MIN_MEMORY) {
        //panic("Insufficient physical memory detected");
    }

    // Calculate bitmap requirements
    total_pages = total_memory / PAGE_SIZE;
    uint64_t bitmap_size = (total_pages + 7) / 8;  // Round up to nearest byte
    bitmap_pages = (bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE;

    // Place bitmap in the largest contiguous usable region
    if (max_usable_size < bitmap_pages * PAGE_SIZE) {
        //panic("No suitable space for bitmap");
    }

    bitmap = (uint8_t*)(max_usable_base + hhdm->offset);
    memset(bitmap, 0xFF, bitmap_pages * PAGE_SIZE);  // Mark all pages as used

    // Reserve memory used by bitmap
    struct limine_memmap_entry bitmap_reserve = {
        .base = max_usable_base,
        .length = bitmap_pages * PAGE_SIZE,
        .type = LIMINE_MEMMAP_RESERVED
    };

    // Mark usable pages as free
    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            uint64_t start = entry->base;
            uint64_t end = start + entry->length;

            // Skip bitmap area
            if (start <= bitmap_reserve.base && end > bitmap_reserve.base) {
                start = bitmap_reserve.base + bitmap_reserve.length;
            }

            uint64_t start_page = start / PAGE_SIZE;
            uint64_t end_page = end / PAGE_SIZE;

            for (uint64_t page = start_page; page < end_page; page++) {
                clear_bit(page);
            }
        }
    }



    debug("PMM initialized: %llu pages, Bitmap at 0x%p (%llu KB)",
        total_pages, bitmap, (bitmap_pages * PAGE_SIZE) / 1024);
}



void* pmm_alloc_pages(size_t count) {
    spinlock_acquire(&pmm_lock);

    uint64_t consecutive = 0;
    uint64_t start_page = last_allocated;

    for (uint64_t i = 0; i < total_pages * 2; i++) {
        uint64_t page = (start_page + i) % total_pages;

        if (!test_bit(page)) {
            if (++consecutive == count) {
                uint64_t first_page = page - count + 1;
                for (uint64_t p = first_page; p <= page; p++) {
                    set_bit(p);
                }
                last_allocated = page + 1;
                spinlock_release(&pmm_lock);
                //                //printk(GFX_CYAN, "Allocated %llu pages at 0x%llx\n", count, first_page * PAGE_SIZE);
                return (void*)(first_page * PAGE_SIZE);
            }
        }
        else {
            consecutive = 0;
        }
    }

    spinlock_release(&pmm_lock);
    return NULL;  // Out of memory
}

void pmm_free_pages(void* addr, size_t count) {
    spinlock_acquire(&pmm_lock);

    uint64_t first_page = (uint64_t)addr / PAGE_SIZE;
    for (uint64_t page = first_page; page < first_page + count; page++) {
        if (page >= total_pages) break;
        clear_bit(page);
    }

    spinlock_release(&pmm_lock);
}

void* pmm_alloc_page(void) {
    return pmm_alloc_pages(1);
}

void pmm_free_page(void* addr) {
    pmm_free_pages(addr, 1);
}