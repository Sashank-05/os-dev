#include <pci/pci.h>

#include <debug/debug.h>
#include <io.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

static struct pci_device devices[PCI_MAX_DEVICES];
static size_t device_count = 0;

static uint32_t pci_config_read32(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    uint32_t address = (uint32_t)(1u << 31)
        | ((uint32_t)bus << 16)
        | ((uint32_t)slot << 11)
        | ((uint32_t)function << 8)
        | (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

static void pci_scan_function(uint8_t bus, uint8_t slot, uint8_t function) {
    if (device_count >= PCI_MAX_DEVICES) {
        return;
    }

    uint32_t header = pci_config_read32(bus, slot, function, 0x00);
    uint16_t vendor_id = header & 0xFFFF;
    if (vendor_id == 0xFFFF) {
        return;
    }

    uint16_t device_id = header >> 16;
    uint32_t class_reg = pci_config_read32(bus, slot, function, 0x08);
    uint8_t class_code = (class_reg >> 24) & 0xFF;
    uint8_t subclass = (class_reg >> 16) & 0xFF;
    uint8_t prog_if = (class_reg >> 8) & 0xFF;
    uint32_t header_reg = pci_config_read32(bus, slot, function, 0x0C);
    uint8_t header_type = (header_reg >> 16) & 0xFF;

    devices[device_count++] = (struct pci_device) {
        .bus = bus,
        .slot = slot,
        .function = function,
        .vendor_id = vendor_id,
        .device_id = device_id,
        .class_code = class_code,
        .subclass = subclass,
        .prog_if = prog_if,
        .header_type = header_type,
    };

    debug("pci %02x:%02x.%x vendor=%04x device=%04x class=%02x subclass=%02x prog_if=%02x\n",
        bus, slot, function, vendor_id, device_id, class_code, subclass, prog_if);
}

static void pci_scan_slot(uint8_t bus, uint8_t slot) {
    uint32_t header = pci_config_read32(bus, slot, 0, 0x0C);
    uint16_t vendor_id = pci_config_read32(bus, slot, 0, 0x00) & 0xFFFF;
    if (vendor_id == 0xFFFF) {
        return;
    }

    uint8_t header_type = (header >> 16) & 0xFF;
    pci_scan_function(bus, slot, 0);

    if (!(header_type & 0x80)) {
        return;
    }

    for (uint8_t function = 1; function < 8; function++) {
        pci_scan_function(bus, slot, function);
    }
}

static void pci_scan_bus(uint8_t bus) {
    for (uint8_t slot = 0; slot < 32; slot++) {
        pci_scan_slot(bus, slot);
    }
}

void pci_init(void) {
    device_count = 0;
    for (uint16_t bus = 0; bus < 256; bus++) {
        pci_scan_bus((uint8_t)bus);
    }

    debug("pci: discovered %zu devices\n", device_count);
}

size_t pci_device_count(void) {
    return device_count;
}

const struct pci_device* pci_device_get(size_t index) {
    if (index >= device_count) {
        return NULL;
    }

    return &devices[index];
}

const struct pci_device* pci_find_class(uint8_t class_code, uint8_t subclass) {
    for (size_t i = 0; i < device_count; i++) {
        if (devices[i].class_code == class_code && devices[i].subclass == subclass) {
            return &devices[i];
        }
    }

    return NULL;
}