#ifndef BGRT_H
#define BGRT_H

#include <stdint.h>
#include <boot/limine.h>
#include <global.h>

// Extern the framebuffer you already have from Limine
extern struct limine_framebuffer *framebuffer;

// Define the Limine RSDP request (to access ACPI tables)
static struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0};

// ACPI System Description Table Header (common for all ACPI tables)
struct acpi_sdt_header
{
    uint32_t signature;        // Table signature (e.g., 'BGRT', 'RSDT', etc.)
    uint32_t length;           // Length of the table, including this header
    uint8_t revision;          // Revision of the table
    uint8_t checksum;          // Checksum of the entire table
    uint8_t oem_id[6];         // OEM ID
    uint64_t oem_table_id;     // OEM Table ID
    uint32_t oem_revision;     // OEM revision number
    uint32_t creator_id;       // Creator ID
    uint32_t creator_revision; // Creator revision number
} __attribute__((packed));

// ACPI BGRT (Boot Graphics Resource Table) structure
typedef struct
{
    struct acpi_sdt_header header; // Common ACPI header
    uint16_t version;              // Version of the image
    uint8_t status;                // Status flags (0: Normal, 1: Invalid)
    uint8_t image_type;            // 0: BMP, 1: JPEG, 2: PNG (in theory, should be BMP)
    uint64_t image_address;        // Address of the image
    uint32_t image_offset_x;       // X offset
    uint32_t image_offset_y;       // Y offset
} __attribute__((packed)) ACPI_BGRT;

struct rsdp_descriptor_2
{
    char signature[8];         // "RSD PTR "
    uint8_t checksum;          // Checksum of the first 20 bytes
    char oem_id[6];            // OEM ID
    uint8_t revision;          // Must be 2 for ACPI 2.0+
    uint32_t rsdt_address;     // 32-bit physical address of the RSDT (for ACPI 1.0)
    uint32_t length;           // Length of the entire table, including this struct
    uint64_t xsdt_address;     // 64-bit physical address of the XSDT (for ACPI 2.0+)
    uint8_t extended_checksum; // Checksum of the entire table
    uint8_t reserved[3];       // Reserved bytes (must be 0)
} __attribute__((packed));

// Function to search for BGRT in ACPI tables
ACPI_BGRT *find_bgrt()
{
    if (rsdp_request.response == NULL || rsdp_request.response->address == NULL)
    {
        return NULL; // No RSDP available, cannot find BGRT
    }

    struct rsdp_descriptor_2 *rsdp = (struct rsdp_descriptor_2 *)rsdp_request.response->address;

    // Check if ACPI version is 2.0 or higher (use XSDT)
    uint64_t *sdt_entries;
    uint64_t sdt_address;
    uint64_t entry_count;

    if (rsdp->revision >= 2 && rsdp->xsdt_address)
    {
        // ACPI 2.0 or higher, use XSDT
        struct acpi_sdt_header *xsdt = (struct acpi_sdt_header *)(uintptr_t)rsdp->xsdt_address;
        sdt_entries = (uint64_t *)(xsdt + 1);
        entry_count = (xsdt->length - sizeof(struct acpi_sdt_header)) / sizeof(uint64_t);
    }
    else
    {
        // ACPI 1.0, use RSDT
        struct acpi_sdt_header *rsdt = (struct acpi_sdt_header *)(uintptr_t)(uint64_t)rsdp->rsdt_address;
        sdt_entries = (uint64_t *)(rsdt + 1);
        entry_count = (rsdt->length - sizeof(struct acpi_sdt_header)) / sizeof(uint32_t);
    }

    // Search for the BGRT table in the SDT entries
    for (uint64_t i = 0; i < entry_count; i++)
    {
        struct acpi_sdt_header *header = (struct acpi_sdt_header *)(uintptr_t)sdt_entries[i];

        // Check if the table signature is "BGRT"
        if (header->signature == 0x54524742)
        { // 'BGRT' in little-endian
            return (ACPI_BGRT *)header;
        }
    }

    return NULL; // BGRT not found
}

// Function to display the vendor logo using the existing framebuffer
void display_vendor_logo()
{
    // Check if framebuffer is available
    if (framebuffer == NULL)
    {
        return; // No framebuffer, abort
    }

    // Find the BGRT table
    ACPI_BGRT *bgrt = find_bgrt();
    if (bgrt == NULL)
    {
        return;
    }

    // Get the framebuffer details
    uint32_t *fb = (uint32_t *)framebuffer->address;
    uint32_t width = framebuffer->width;
    uint32_t height = framebuffer->height;

    // Assuming BGRT provides a BMP (for simplicity, handling BMP format here)
    uint8_t *image = (uint8_t *)bgrt->image_address;
    uint32_t image_width = *((uint32_t *)(image + 18));        // Width from BMP header
    uint32_t image_height = *((uint32_t *)(image + 22));       // Height from BMP header
    uint8_t *pixel_data = image + *((uint32_t *)(image + 10)); // Pixel data offset

    // BGRT provides offsets to place the image
    uint32_t offset_x = bgrt->image_offset_x;
    uint32_t offset_y = bgrt->image_offset_y;

    // Drawing the logo onto the framebuffer
    for (uint32_t y = 0; y < image_height; ++y)
    {
        for (uint32_t x = 0; x < image_width; ++x)
        {
            uint32_t bmp_index = (x + y * image_width) * 3;
            uint8_t blue = pixel_data[bmp_index];
            uint8_t green = pixel_data[bmp_index + 1];
            uint8_t red = pixel_data[bmp_index + 2];

            uint32_t framebuffer_index = (x + offset_x) + (height - (y + offset_y)) * width;
            fb[framebuffer_index] = (red << 16) | (green << 8) | blue; // RGB format for framebuffer
        }
    }
}

#endif // BGRT_H
