#include <limine.h>
#include <common/stdinfo.h>
#include <drivers/framebuffer.h>
#include <debug/debug.h>
#include <debug/panic.h>
#include <memory/paging.h>
#include <drivers/acpi.h>
#include <string.h>

extern struct GFX_Struct gfx;
extern struct OSinfo osinfo;


void map_physical_memory(uint64_t phys_addr, size_t size) {
    uint64_t virt_addr = phys_addr + osinfo.hhdm;
    uint64_t num_pages = (size + 0xFFF) / 0x1000;
    map_pages((uint64_t*)(osinfo.cr3 + osinfo.hhdm), virt_addr, phys_addr, num_pages, KERNEL_PFLAG_PRESENT);
}

// BMP Header Structure
typedef struct {
    char signature[2];
    uint32_t file_size;
    uint32_t reserved;
    uint32_t data_offset;
    uint32_t header_size;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bpp;
    uint32_t compression;
    uint32_t image_size;
} __attribute__((packed)) BMPHeader;

void _bgrt(BGRT* bgrt) {
    uint64_t image_address = bgrt->image_address;
    uint32_t x_offset = bgrt->image_x_offset;
    uint32_t y_offset = bgrt->image_y_offset;

    map_physical_memory(image_address, 0x1000);

    BMPHeader* bmp_header = (BMPHeader*)(image_address + osinfo.hhdm);
    if (bmp_header->signature[0] != 'B' || bmp_header->signature[1] != 'M') {
        log(ERROR, "Invalid BMP signature\n");
        return;
    }

    uint32_t width = bmp_header->width;
    uint32_t height = bmp_header->height;
    uint16_t bpp = bmp_header->bpp;
    uint32_t data_offset = bmp_header->data_offset;
    uint32_t image_size = bmp_header->image_size;

    if (image_size == 0) {
        uint32_t row_size = ((bpp * width + 31) / 32) * 4;
        image_size = row_size * height;
    }
    
    map_physical_memory(image_address, image_size);

    uint8_t* pixel_data = (uint8_t*)(image_address + osinfo.hhdm) + data_offset;
    uint32_t stride = ((bpp * width + 31) / 32) * 4;

    for (int y = height - 1; y >= 0; y--) {
        uint8_t* row = pixel_data + (y * stride);
        for (uint32_t x = 0; x < width; x++) {
            uint32_t pixel = 0;

            // Read pixel based on BPP
            if (bpp == 32) {
                uint8_t a = row[x * 4 + 3];
                uint8_t r = row[x * 4 + 2];
                uint8_t g = row[x * 4 + 1];
                uint8_t b = row[x * 4 + 0];
                pixel = (r << 8) | (g << 16) | (b << 24) | a;
            } else if (bpp == 24) {
                uint8_t b_val = row[x * 3 + 0];
                uint8_t g_val = row[x * 3 + 1];
                uint8_t r_val = row[x * 3 + 2];
                pixel = 0xFF000000 | (b_val << 16) | (g_val << 8) | r_val;
            } else {
                continue;
            }

            uint32_t fb_pixel = ((pixel & 0xFF000000)) |
                                ((pixel & 0x00FF0000) >> 16) |
                                ((pixel & 0x0000FF00)) |
                                ((pixel & 0x000000FF) << 16);
            framebuffer_draw_pixel(x_offset + x, y_offset + (height - y - 1), fb_pixel);
        }
    }
}

void _fadt(FADT* facp){

}

bool CheckSig(char* sig, char* csig){
    if(strncmp(sig, csig, 4) == 0){
        return 1;
    } else {
        return 0;
    }
}

void init_acpi(void) {
    if (osinfo.rsdp == NULL) {
        panic("No RSDP found\n");
        return;
    }

    map_pages((uint64_t*)(osinfo.cr3 + osinfo.hhdm), (uint64_t)osinfo.rsdp_response->address, ((uint64_t)osinfo.rsdp_response->address) - osinfo.hhdm, 1, KERNEL_PFLAG_PRESENT);
    if (osinfo.rsdp_response->revision != 0) {
        panic("Unsupported ACPI version\n");
        return;
    }

    map_pages((uint64_t*)(osinfo.cr3 + osinfo.hhdm), (uint64_t)osinfo.rsdp->rsdt_address + osinfo.hhdm, (uint64_t)osinfo.rsdp->rsdt_address, 1, KERNEL_PFLAG_PRESENT);
    RSDT* rsdt = (RSDT*)(osinfo.rsdp->rsdt_address + osinfo.hhdm);
    size_t num_entries = (rsdt->header.length - sizeof(rsdt->header)) / 4;

    for (size_t i = 0; i < num_entries; i++) {
        map_pages((uint64_t*)(osinfo.cr3 + osinfo.hhdm), (uint64_t)rsdt->entries[i] + osinfo.hhdm, (uint64_t)rsdt->entries[i], 1, KERNEL_PFLAG_PRESENT);
        ISDTHeader* header = (ISDTHeader*)(rsdt->entries[i] + osinfo.hhdm);
        char sig[5] = { 0 };
        memcpy(sig, header->signature, 4);
        
        printk(GFX_WHITE, "Entry %d: %s \n", i, sig);


        if (CheckSig(header->signature, "BGRT")) {
            _bgrt((BGRT*)header);
        } else if (CheckSig(header->signature, "FACP")){
            _fadt((FADT*)header);
        }
    }

    
}
