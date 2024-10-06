#include <mem.h>

void *memcpy(void *dest, const void *src, uint32_t n) {
    const uint8_t *sp = (const uint8_t *)src;
    uint8_t *dp = (uint8_t *)dest;
    for (uint32_t i = 0; i < n; i++) dp[i] = sp[i];
    return dest;
}

void *memset(void *dest, uint8_t val, uint32_t n) {
    uint8_t *temp = (uint8_t *)dest;
    for (uint32_t i = 0; i < n; i++) temp[i] = val;
    return dest;
}

uint16_t *memsetw(uint16_t *dest, uint16_t val, uint32_t n) {
    uint16_t *temp = (uint16_t *)dest;
    for (uint32_t i = 0; i < n; i++) temp[i] = val;
    return dest;
}

