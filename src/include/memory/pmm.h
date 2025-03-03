#ifndef _PMM_H
#define _PMM_H

#include <stddef.h>

// Physical Memory Manager (PMM) initialization
void pmm_init(void);

// Allocate multiple contiguous pages
// Returns physical address of first page, or NULL if allocation fails
void* pmm_alloc_pages(size_t count);

// Free multiple contiguous pages starting at the given physical address
void pmm_free_pages(void* addr, size_t count);

// Allocate a single page
// Returns physical address of the page, or NULL if allocation fails
void* pmm_alloc_page(void);

// Free a single page at the given physical address
void pmm_free_page(void* addr);

#endif // _PMM_H