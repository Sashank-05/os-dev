#ifndef MEM_H
#define MEM_H

#include <stdint.h>

void *memcpy(void *dest, const void *src, uint32_t n);
void *memset(void *dest, uint8_t val, uint32_t n);
uint16_t *memsetw(uint16_t *dest, uint16_t val, uint32_t n);

#endif