#include <common/gdt.h>

extern void init_gdt(void);

void gdt_init() {
    init_gdt();
}