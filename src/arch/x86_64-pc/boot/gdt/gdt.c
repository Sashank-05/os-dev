#include <common/gdt.h>

extern void init_gdt(void);
extern void init_tss(uint64_t rsp0);

void gdt_init() {
    init_gdt();
}

void tss_init(uint64_t rsp0) {
    init_tss(rsp0);
}