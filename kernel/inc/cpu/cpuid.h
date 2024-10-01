#ifndef CPUID_H
#define CPUID_H

#include <stdint.h>

typedef struct {
    char vendor[13];
    uint32_t eax, ebx, ecx, edx;
} cpu_info_t;

void get_cpu_info(cpu_info_t* info);

#endif
