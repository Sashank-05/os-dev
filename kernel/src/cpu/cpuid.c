#include <cpu/cpuid.h>

void get_cpu_info(cpu_info_t* info) {
    // Prepare the buffer to store the vendor string
    uint32_t ebx = 0, ecx = 0, edx = 0;

    // Get vendor string and CPUID function 0 information
    __asm__ volatile(
        "cpuid"
        : "=a" (info->eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
        : "a" (0)  // Input EAX = 0 to get CPU vendor
    );

    // Store the vendor string (stored in EBX, EDX, ECX)
    *(uint32_t*)(info->vendor + 0) = ebx;
    *(uint32_t*)(info->vendor + 4) = edx;
    *(uint32_t*)(info->vendor + 8) = ecx;
    info->vendor[12] = '\0';  // Null-terminate the vendor string
}
