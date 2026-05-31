#include <drivers/framebuffer.h>
#include <common/stdinfo.h>
#include <stdbool.h>
#include <stdint.h>
#include <libk/string.h>
#include <memory/paging.h>
#include <memory/pmm.h>
#include <debug/panic.h>
#include <debug/debug.h>

extern struct OSinfo osinfo;

#define PAGE_SIZE 4096

#define PAGE_ALIGN_DOWN(addr) ((addr) & ~(PAGE_SIZE - 1))
#define PAGE_ALIGN_UP(addr)   (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

#define TOPBITS 0xFFFF000000000000

static inline void halt(void) {
//    printk(GFX_RED, "Error occurred, halted\n");
    asm volatile("hlt");
}

/*
 * Convert a virtual address to its physical address using a 4‐level page table.
 * Returns 0xDEAD if any level is unmapped.
 */
uint64_t virt_to_phys(uint64_t* pml4, uint64_t virt_addr) {
    /* Clear any high bits (if necessary) */
    virt_addr &= ~TOPBITS;

    /* Calculate the indices for the 4 levels.
     * (Using 0x1FF == 511 for the 9-bit mask.)
     */
    uint64_t pml4_index = (virt_addr >> 39) & 0x1FF;
    uint64_t pml3_index = (virt_addr >> 30) & 0x1FF;
    uint64_t pml2_index = (virt_addr >> 21) & 0x1FF;
    uint64_t pml1_index = (virt_addr >> 12) & 0x1FF;

    if (!(pml4[pml4_index] & KERNEL_PFLAG_PRESENT))
        return 0xDEAD;
    uint64_t* pml3 = (uint64_t*)(PAGE_ALIGN_DOWN(pml4[pml4_index]) + osinfo.hhdm);

    if (!(pml3[pml3_index] & KERNEL_PFLAG_PRESENT))
        return 0xDEAD;
    uint64_t* pml2 = (uint64_t*)(PAGE_ALIGN_DOWN(pml3[pml3_index]) + osinfo.hhdm);

    if (!(pml2[pml2_index] & KERNEL_PFLAG_PRESENT))
        return 0xDEAD;
    uint64_t* pml1 = (uint64_t*)(PAGE_ALIGN_DOWN(pml2[pml2_index]) + osinfo.hhdm);

    if (!(pml1[pml1_index] & KERNEL_PFLAG_PRESENT))
        return 0xDEAD;

    /* Return the physical base plus the offset within the page */
    return (PAGE_ALIGN_DOWN(pml1[pml1_index]) | (virt_addr & (PAGE_SIZE - 1)));
}

/*
 * Write data to virtual memory.
 * Walks the page tables to locate the kernel's virtual mapping.
 */
void write_vmem(uint64_t* pml4, uint64_t virt_addr, char* data, size_t len) {
    while (len > 0) {
        uint64_t phys_addr = virt_to_phys(pml4, virt_addr);
        if (phys_addr == 0xDEAD) {
            printk(GFX_RED, "write_vmem: virtual address is not mapped! Address: 0x%x\n"
                "         Cannot write to vmem. Halting.\n", virt_addr);
            halt();
        }
        /* Convert physical address to kernel (direct‐mapped) address */
        uint64_t kernel_addr = phys_addr + osinfo.hhdm;

        size_t bytes_to_copy = (len < PAGE_SIZE) ? len : PAGE_SIZE;
        memcpy((char*)kernel_addr, data, bytes_to_copy);

        len -= bytes_to_copy;
        virt_addr += bytes_to_copy;
        data += bytes_to_copy;
    }
}

void push_vmem(uint64_t* pml4, uint64_t rsp, char* data, size_t len) {
    rsp -= len;
    write_vmem(pml4, rsp, data, len);
}

/*
 * Map a series of pages starting at a given virtual address.
 * If any intermediate page table is not allocated, allocate and clear it.
 */
void map_pages(uint64_t* pml4, uint64_t vaddr, uint64_t paddr, uint64_t num_pages, uint64_t flags) {
    /* Make sure the virtual address is page-aligned and clear high bits */
    vaddr = PAGE_ALIGN_DOWN(vaddr & ~TOPBITS);

    /* Calculate initial indices */
    uint64_t pml4_index = (vaddr >> 39) & 0x1FF;
    uint64_t pml3_index = (vaddr >> 30) & 0x1FF;
    uint64_t pml2_index = (vaddr >> 21) & 0x1FF;
    uint64_t pml1_index = (vaddr >> 12) & 0x1FF;

    while (num_pages > 0) {
        /* Ensure each level of the page table exists; allocate if not. */
        if (!(pml4[pml4_index] & KERNEL_PFLAG_PRESENT)) {
            uint64_t new_page = (uint64_t)pmm_alloc_pages(1);
            memset((void*)(new_page + osinfo.hhdm), 0, PAGE_SIZE);
            pml4[pml4_index] = new_page | flags | KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE | KERNEL_PFLAG_USER;
        }
        uint64_t* pml3 = (uint64_t*)(PAGE_ALIGN_DOWN(pml4[pml4_index]) + osinfo.hhdm);

        if (!(pml3[pml3_index] & KERNEL_PFLAG_PRESENT)) {
            uint64_t new_page = (uint64_t)pmm_alloc_pages(1);
            memset((void*)(new_page + osinfo.hhdm), 0, PAGE_SIZE);
            pml3[pml3_index] = new_page | flags | KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE | KERNEL_PFLAG_USER;
        }
        uint64_t* pml2 = (uint64_t*)(PAGE_ALIGN_DOWN(pml3[pml3_index]) + osinfo.hhdm);

        if (!(pml2[pml2_index] & KERNEL_PFLAG_PRESENT)) {
            uint64_t new_page = (uint64_t)pmm_alloc_pages(1);
            memset((void*)(new_page + osinfo.hhdm), 0, PAGE_SIZE);
            pml2[pml2_index] = new_page | flags | KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE | KERNEL_PFLAG_USER;
        }
        uint64_t* pml1 = (uint64_t*)(PAGE_ALIGN_DOWN(pml2[pml2_index]) + osinfo.hhdm);

        /* Map the page */
        pml1[pml1_index] = paddr | flags;

        /* Advance to the next page */
        num_pages--;
        vaddr += PAGE_SIZE;
        paddr += PAGE_SIZE;

        /* Increment the table indices appropriately with rollover */
        pml1_index++;
        if (pml1_index == 512) {
            pml1_index = 0;
            pml2_index++;
            if (pml2_index == 512) {
                pml2_index = 0;
                pml3_index++;
                if (pml3_index == 512) {
                    pml3_index = 0;
                    pml4_index++;
                    if (pml4_index == 512) {
//                        printk(GFX_WHITE, "Failed to allocate pages: Out of virtual memory.\n");
                        halt();
                    }
                }
            }
        }
    }
}

/*
 * Allocate new physical pages for a range of virtual addresses.
 * This function is similar to map_pages() but calls pmm_alloc_pages(1) for each mapping.
 */
void alloc_pages(uint64_t* pml4, uint64_t vaddr, uint64_t num_pages, uint64_t flags) {
    vaddr = PAGE_ALIGN_DOWN(vaddr & ~TOPBITS);
    uint64_t pml4_index = (vaddr >> 39) & 0x1FF;
    uint64_t pml3_index = (vaddr >> 30) & 0x1FF;
    uint64_t pml2_index = (vaddr >> 21) & 0x1FF;
    uint64_t pml1_index = (vaddr >> 12) & 0x1FF;

    while (num_pages > 0) {
        if (!(pml4[pml4_index] & KERNEL_PFLAG_PRESENT)) {
            uint64_t new_page = (uint64_t)pmm_alloc_pages(1);
//            printk(GFX_CYAN, "Allocated new page at 0x%x\n", new_page+osinfo.hhdm);
            memset((void*)(new_page + osinfo.hhdm), 0, PAGE_SIZE);
            pml4[pml4_index] = new_page | flags | KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE | KERNEL_PFLAG_USER;
        }
        uint64_t* pml3 = (uint64_t*)(PAGE_ALIGN_DOWN(pml4[pml4_index]) + osinfo.hhdm);

        if (!(pml3[pml3_index] & KERNEL_PFLAG_PRESENT)) {
            uint64_t new_page = (uint64_t)pmm_alloc_pages(1);
            memset((void*)(new_page + osinfo.hhdm), 0, PAGE_SIZE);
            pml3[pml3_index] = new_page | flags | KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE | KERNEL_PFLAG_USER;
        }
        uint64_t* pml2 = (uint64_t*)(PAGE_ALIGN_DOWN(pml3[pml3_index]) + osinfo.hhdm);

        if (!(pml2[pml2_index] & KERNEL_PFLAG_PRESENT)) {
            uint64_t new_page = (uint64_t)pmm_alloc_pages(1);
            memset((void*)(new_page + osinfo.hhdm), 0, PAGE_SIZE);
            pml2[pml2_index] = new_page | flags | KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE | KERNEL_PFLAG_USER;
        }
        uint64_t* pml1 = (uint64_t*)(PAGE_ALIGN_DOWN(pml2[pml2_index]) + osinfo.hhdm);

        /* Allocate a new physical page for this mapping */
        uint64_t new_page = (uint64_t)pmm_alloc_pages(1);
        memset((void*)(new_page + osinfo.hhdm), 0, PAGE_SIZE);
        pml1[pml1_index] = new_page | flags;

//        printk(GFX_CYAN, "Allocated new page at 0x%x\n", new_page+osinfo.hhdm);
        num_pages--;
        vaddr += PAGE_SIZE;

        pml1_index++;
        if (pml1_index == 512) {
            pml1_index = 0;
            pml2_index++;
            if (pml2_index == 512) {
                pml2_index = 0;
                pml3_index++;
                if (pml3_index == 512) {
                    pml3_index = 0;
                    pml4_index++;
                    if (pml4_index == 512) {
//                        printk(GFX_RED, "Failed to allocate pages: Out of virtual memory.\n");
                        halt();
                    }
                }
            }
        }
    }
}

/*
 * Invalidate a single page from the CPU’s TLB.
 */
void clear_page_cache(uint64_t addr) {
    asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}

/* External kernel section markers */
extern uint64_t p_kernel_start[];
extern uint64_t p_writeallowed_start[];
extern uint64_t p_kernel_end[];

uint64_t kernel_start = (uint64_t)p_kernel_start;
uint64_t writeallowed_start = (uint64_t)p_writeallowed_start;
uint64_t kernel_end = (uint64_t)p_kernel_end;

/*
 * Map sections using the memory map provided by limine.
 */
void map_sections(uint64_t* pml4) {
//    printk(GFX_WHITE, "Mapping sections\n");
    uint64_t num_memmap_entries = osinfo.memmap->entry_count;
//    printk(GFX_WHITE, "Mapping %d memory map entries\n", num_memmap_entries);
    struct limine_memmap_entry** memmap_entries = osinfo.memmap->entries;
    for (size_t entry = 0; entry < num_memmap_entries; entry++) {
        uint64_t entry_type = memmap_entries[entry]->type;
        if (entry_type == LIMINE_MEMMAP_USABLE ||
            entry_type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE ||
            entry_type == LIMINE_MEMMAP_ACPI_NVS ||
            entry_type == LIMINE_MEMMAP_FRAMEBUFFER ||
            entry_type == LIMINE_MEMMAP_KERNEL_AND_MODULES ||
            entry_type == LIMINE_MEMMAP_ACPI_RECLAIMABLE) {

           /* printk(GFX_WHITE, "Mapped entry %llu of type %llu: %llu bytes at 0x%llx\n",
                (unsigned long long)entry,
                (unsigned long long)memmap_entries[entry]->type,
                (unsigned long long)memmap_entries[entry]->length,
                (unsigned long long)memmap_entries[entry]->base);
             The virtual address is set to physical base + HHDM offset */
            uint64_t flags = KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE;
            if (entry_type == LIMINE_MEMMAP_FRAMEBUFFER) {
                flags |= KERNEL_PFLAG_USER;
            }
            map_pages(pml4, memmap_entries[entry]->base + osinfo.hhdm, memmap_entries[entry]->base, memmap_entries[entry]->length / 4096, flags);

        }
    }
}

/*
 * Map the kernel:
 *  - Map the read-only portion with just PRESENT.
 *  - Map the writable portion with PRESENT and WRITE.
 *  - Allocate a kernel stack.
 */
void map_kernel(uint64_t* pml4) {
    uint64_t length;
    uint64_t phys_base;

    /* Map from kernel_start to writeallowed_start (read-only) */
    length = PAGE_ALIGN_UP(writeallowed_start - kernel_start);
    phys_base = osinfo.kernel_addr.physical_base + (kernel_start - osinfo.kernel_addr.virtual_base);
    map_pages(pml4, PAGE_ALIGN_DOWN(kernel_start), phys_base, length / PAGE_SIZE, KERNEL_PFLAG_PRESENT);

    /* Map from writeallowed_start to kernel_end (writable) */
//    printk(GFX_WHITE, "Mapping kernel from 0x%x to 0x%x\n", writeallowed_start, kernel_end);
    length = PAGE_ALIGN_UP(kernel_end - writeallowed_start);
    phys_base = osinfo.kernel_addr.physical_base + (writeallowed_start - osinfo.kernel_addr.virtual_base);
    map_pages(pml4, PAGE_ALIGN_DOWN(writeallowed_start), phys_base, length / PAGE_SIZE,
        KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE);

    /* Map the kernel's stack */
    if (osinfo.cr3 != 0) {
        uint64_t* current_pml4 = (uint64_t*)(osinfo.cr3 + osinfo.hhdm);
        for (uint64_t i = 0; i < KERNEL_STACK_PAGES; i++) {
            uint64_t vaddr = KERNEL_STACK_ADDR + i * PAGE_SIZE;
            uint64_t paddr = virt_to_phys(current_pml4, vaddr);
            map_pages(pml4, vaddr, paddr, 1, KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE);
        }
    } else {
        alloc_pages(pml4, KERNEL_STACK_ADDR, KERNEL_STACK_PAGES, KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE);
    }
//    printk(GFX_WHITE, "Mapped kernel stack at 0x%x\n", KERNEL_STACK_ADDR);
}

/*
 * Map the entire address space required by the kernel.
 */
void map_all(uint64_t* pml4) {
    map_kernel(pml4);
//    printk(GFX_WHITE, "Mapped kernel\n");
    map_sections(pml4);
//    printk(GFX_WHITE, "Mapped sections\n");
}

/*
 * Initialize a paging tree for a new task.
 */
uint64_t* init_paging_task(void) {
    uint64_t* pml4_virt = (uint64_t*)((uint64_t)pmm_alloc_pages(1) + osinfo.hhdm);
    memset((void*)pml4_virt, 0, PAGE_SIZE);
    map_all(pml4_virt);
    return pml4_virt;
}

/*
 * Initialize the kernel’s paging tree.
 */
void init_paging(void) {
//    printk(GFX_BLUE, "Creating page tree... \n");
    uint64_t* pml4_virt = (uint64_t*)((uint64_t)pmm_alloc_pages(1) + osinfo.hhdm);
    memset((void*)pml4_virt, 0, PAGE_SIZE);
    map_all(pml4_virt);
//    printk(GFX_BLUE, "Page tree created\n");
//    printk(GFX_BLUE, "PML4 PHY: 0x%llx\n", (unsigned long long)pml4_virt-osinfo.hhdm);
//    printk(GFX_BLUE, "PML4 VIRT: 0x%llx\n", (unsigned long long)pml4_virt);

    osinfo.cr3 = (unsigned long long)pml4_virt - osinfo.hhdm;
//    printk(GFX_BLUE, "CR3: 0x%llx\n", osinfo.cr3);
//    printk(GFX_BLUE, "HHDM: 0x%llx\n", osinfo.hhdm);
    __asm__ volatile (
        "movq %cr3, %rax\n\t"
        "movq %rax, %cr3\n\t" // TLB Flush
        );
    debug("\nCR3 switched & TLB flushed\n");
    
    // test cr3[0] = 0x0000000000000000

    for (int i = 0; i < 512; i++) {
        if (pml4_virt[i] != 0) {
//            printk(GFX_WHITE, "PML4[%d]: 0x%llx\n", i, pml4_virt[i]);
        }
    }

}
