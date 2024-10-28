#ifndef GRAPHICS_BOOT_H
#define GRAPHICS_BOOT_H

#include <global.h>
#include <boot/limine.h> 
#include <graphics/err.h>

static inline int init_graphics(void) {
    if (framebuffer == NULL) {
        return ERR_INVALID_FRAMEBUFFER;
    } elif (framebuffer->bpp != 32) {
        return ERR_UNSUPPORTED_BPP;
    }
    return 0;
};

#endif // GRAPHICS_BOOT_H