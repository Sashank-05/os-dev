#ifndef _BGRT_H
#define _BGRT_H

#include <stdint.h>
#include <stdbool.h>
#include <boot/limine.h>

extern struct limine_framebuffer *framebuffer;

// Add Limine RSDP request
static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0};

// BGRT Structure as per ACPI Specification
struct acpi_bgrt
{
    // Header
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;

    // BGRT specific fields
    uint16_t version;
    uint8_t status;
    uint8_t image_type; // 0 = Bitmap
    uint64_t image_address;
    uint32_t image_offset_x;
    uint32_t image_offset_y;
} __attribute__((packed));

// XSDT structure
struct acpi_xsdt
{
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
    uint64_t entries[];
} __attribute__((packed));

// RSDT structure
struct acpi_rsdt
{
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
    uint32_t entries[];
} __attribute__((packed));

// Bitmap header structure
struct bmp_header
{
    uint16_t signature;
    uint32_t file_size;
    uint32_t reserved;
    uint32_t data_offset;
    uint32_t header_size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bpp;
    uint32_t compression;
    uint32_t image_size;
    int32_t x_pixels_per_meter;
    int32_t y_pixels_per_meter;
    uint32_t colors_used;
    uint32_t important_colors;
} __attribute__((packed));

// Get RSDP from Limine
static inline void *find_rsdp(void)
{
    if (rsdp_request.response == NULL)
    {
        return NULL;
    }
    return (void *)rsdp_request.response->address;
}

// Find BGRT table through XSDT/RSDT
static inline struct acpi_bgrt *find_bgrt(void)
{
    void *rsdp = find_rsdp();
    if (!rsdp)
        return NULL;

    // Check if we're using XSDT (ACPI 2.0+) or RSDT
    struct acpi_xsdt *xsdt;
    struct acpi_rsdt *rsdt;
    uint8_t *rsdp_ptr = (uint8_t *)rsdp;

    // If revision >= 2, use XSDT
    if (rsdp_ptr[15] >= 2 && *(uint64_t *)(rsdp_ptr + 24) != 0)
    {
        xsdt = (struct acpi_xsdt *)(uint64_t)(*(uint64_t *)(rsdp_ptr + 24));
        size_t entries = (xsdt->length - sizeof(struct acpi_xsdt)) / 8;

        // Search through XSDT entries
        for (size_t i = 0; i < entries; i++)
        {
            struct acpi_bgrt *table = (struct acpi_bgrt *)(uint64_t)xsdt->entries[i];
            if (table->signature[0] == 'B' && table->signature[1] == 'G' &&
                table->signature[2] == 'R' && table->signature[3] == 'T')
            {
                return table;
            }
        }
    }
    else
    {
        // Fall back to RSDT for older ACPI versions
        rsdt = (struct acpi_rsdt *)(uint64_t)(*(uint32_t *)(rsdp_ptr + 16));
        size_t entries = (rsdt->length - sizeof(struct acpi_rsdt)) / 4;

        // Search through RSDT entries
        for (size_t i = 0; i < entries; i++)
        {
            struct acpi_bgrt *table = (struct acpi_bgrt *)(uint64_t)rsdt->entries[i];
            if (table->signature[0] == 'B' && table->signature[1] == 'G' &&
                table->signature[2] == 'R' && table->signature[3] == 'T')
            {
                return table;
            }
        }
    }

    return NULL;
}

// Convert BMP 24-bit color to framebuffer format
static inline uint32_t convert_color(uint8_t b, uint8_t g, uint8_t r)
{
    // Assuming 32-bit RGBA
    return (r << 16) | (g << 8) | b;
}

// Display vendor logo function
void display_vendor_logo(void)
{
    struct acpi_bgrt *bgrt = find_bgrt();
    if (!bgrt || !bgrt->image_address)
    {
        for (uint64_t y = 0; y < (uint64_t) (framebuffer->height)/4; y++)
        {
            for (uint64_t x = 0; x < (uint64_t) (framebuffer->width)/4; x++)
            {
                //draw_pixel(x, y, 0xFF0000);
            }
        }
        return;
    }

    struct bmp_header *bmp = (struct bmp_header *)bgrt->image_address;
    if (bmp->signature != 0x4D42)
        return; // Check BM signature

    // Get pointer to pixel data
    uint8_t *pixel_data = (uint8_t *)bgrt->image_address + bmp->data_offset;

    // Calculate center position if offsets are 0
    uint32_t start_x = bgrt->image_offset_x;
    uint32_t start_y = bgrt->image_offset_y;

    if (start_x == 0)
    {
        start_x = (framebuffer->width - bmp->width) / 2;
    }
    if (start_y == 0)
    {
        start_y = (framebuffer->height - bmp->height) / 2;
    }

    // BMP is stored bottom-up
    for (int32_t y = bmp->height - 1; y >= 0; y--)
    {
        for (int32_t x = 0; x < bmp->width; x++)
        {
            size_t pixel_index = (y * ((bmp->width * 3 + 3) & ~3) + x * 3);
            uint8_t b = pixel_data[pixel_index];
            uint8_t g = pixel_data[pixel_index + 1];
            uint8_t r = pixel_data[pixel_index + 2];

            uint32_t color = convert_color(b, g, r);
            draw_pixel(start_x + x, start_y + (bmp->height - 1 - y), color);
        }
    }
}

#endif // _BGRT_H