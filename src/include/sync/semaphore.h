#pragma once

#ifndef SEMAPHORE_H
#define SEMAPHORE_H
#include <stddef.h>


struct Semaphore
{
    size_t count;
    size_t max_count;
};

void semaphore_init(struct Semaphore* semaphore, size_t max_count);
void semaphore_wait(struct Semaphore* semaphore);

#endif
