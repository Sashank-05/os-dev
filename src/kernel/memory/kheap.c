#include <stdint.h>
#include <stdbool.h>
#include <common/stdinfo.h>
#include <drivers/framebuffer.h>
#include <memory/pmm.h>
#include <memory/kheap.h>
#include <io.h>

#define KPOOL_INTEGRITY 0xCAFE
#define ALIGNMENT 16
#define MIN_POOL_SIZE (sizeof(kPool_t) + ALIGNMENT)

typedef struct kPool {
    uint16_t integrity;
    uint64_t total_size;     // Total size including metadata
    uint64_t allocated_size; // User-requested size
    bool is_free;
    struct kPool* next;
} kPool_t;

static kPool_t* first_pool = NULL;

// Alignment functions
static inline uint64_t align_up(uint64_t n) {
    return (n + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
}

static inline void* alloc_from_pool(kPool_t* pool) {
    return (void*)((uintptr_t)pool + sizeof(kPool_t));
}

void kheap_init(void) {
//    printk(GFX_WHITE, "Initializing kernel heap\n");
    
    // Allocate initial heap space (4MB)
    uint64_t pages = 1024;
    uintptr_t phys = pmm_alloc_pages(pages);
    
    if (!phys) {
//        printk(GFX_RED, "Failed to allocate physical pages for heap\n");
        for(;;);
    }
    
    first_pool = (kPool_t*)(phys + osinfo.hhdm);
    uint64_t heap_size = pages * 4096 - sizeof(kPool_t);
    
    *first_pool = (kPool_t){
        .integrity = KPOOL_INTEGRITY,
        .total_size = heap_size,
        .allocated_size = 0,
        .is_free = true,
        .next = NULL
    };
    
    osinfo.kheap = (uintptr_t)first_pool;
//    printk(GFX_WHITE, "Kernel heap initialized at 0x%llx\n", osinfo.kheap);
}

static kPool_t* find_free_pool(uint64_t size) {
    kPool_t* current = first_pool;
    while (current) {
        if (current->is_free && current->total_size >= size + sizeof(kPool_t)) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static kPool_t* create_new_pool(uint64_t size) {
    uint64_t pages = (size + 4095) / 4096;
    uintptr_t phys = pmm_alloc_pages(pages);
    
    if (!phys) return NULL;
    
    kPool_t* new_pool = (kPool_t*)(phys + osinfo.hhdm);
    *new_pool = (kPool_t){
        .integrity = KPOOL_INTEGRITY,
        .total_size = pages * 4096 - sizeof(kPool_t),
        .allocated_size = 0,
        .is_free = true,
        .next = NULL
    };
    
    return new_pool;
}

static void split_pool(kPool_t* pool, uint64_t size) {
    uint64_t remaining = pool->total_size - size - sizeof(kPool_t);
    if (remaining < MIN_POOL_SIZE) return;

    kPool_t* new_pool = (kPool_t*)((uintptr_t)pool + sizeof(kPool_t) + size);
    *new_pool = (kPool_t){
        .integrity = KPOOL_INTEGRITY,
        .total_size = remaining,
        .allocated_size = 0,
        .is_free = true,
        .next = pool->next
    };
    
    pool->total_size = size;
    pool->next = new_pool;
}

void* kmalloc(uint64_t size) {
    if (size == 0) return NULL;
    
    size = align_up(size);
    kPool_t* pool = find_free_pool(size);
    
    if (!pool) {
        pool = create_new_pool(size);
        if (!pool) return NULL;
        
        // Add to end of pool list
        kPool_t* current = first_pool;
        while (current->next) current = current->next;
        current->next = pool;
    }
    
    if (pool->total_size >= size + sizeof(kPool_t) + MIN_POOL_SIZE) {
        split_pool(pool, size);
    }
    
    pool->is_free = false;
    pool->allocated_size = size;
    return alloc_from_pool(pool);
}

static void merge_pools(kPool_t* prev, kPool_t* current) {
    if (prev && prev->is_free && (uintptr_t)prev + sizeof(kPool_t) + prev->allocated_size == (uintptr_t)current) {
        prev->total_size += sizeof(kPool_t) + current->total_size;
        prev->next = current->next;
    }
}

void kfree(void* ptr) {
    if (!ptr) return;
    
    kPool_t* pool = (kPool_t*)((uintptr_t)ptr - sizeof(kPool_t));
    if (pool->integrity != KPOOL_INTEGRITY) {
//        printk(GFX_RED, "Invalid free: Pool integrity check failed\n");
        for(;;);
    }
    
    pool->is_free = true;
    kPool_t* current = first_pool;
    kPool_t* prev = NULL;
    
    // Merge adjacent free blocks
    while (current) {
        if (current->is_free) {
            merge_pools(prev, current);
        }
        prev = current;
        current = current->next;
    }
}