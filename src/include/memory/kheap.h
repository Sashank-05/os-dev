#ifndef _KHEAP_H
#define _KHEAP_H

#include <stdint.h>
#include <stdbool.h>
#include <common/stdinfo.h>

extern struct OSinfo osinfo;


#define KPOOL_INTEGRITY 0xCAFE


void kheap_init(void);

void* kmalloc(uint64_t size);

void kfree(void* ptr);

#endif
