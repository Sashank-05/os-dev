#include <sync/spinlock.h>

#include <stdint.h>

#define SPINLOCK_INIT {0}

void spinlock_acquire(spinlock_t* lock) {
    while (__sync_lock_test_and_set(&lock->lock, 1)) {
        // Spin
    }
}

void spinlock_release(spinlock_t* lock) {
    __sync_lock_release(&lock->lock);
}
