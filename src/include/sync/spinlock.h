#ifndef _SPINLOCK_H
#define _SPINLOCK_H


#include <stdint.h>

typedef struct {
    volatile uint8_t lock;
} spinlock_t;

#define SPINLOCK_INIT {0}

void spinlock_acquire(spinlock_t* lock);
void spinlock_release(spinlock_t* lock);

#endif // _SPINLOCK_H