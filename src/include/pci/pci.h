#pragma once

#include <stddef.h>
#include <stdint.h>

#define PCI_MAX_DEVICES 64

struct pci_device {
    uint8_t bus;
    uint8_t slot;
    uint8_t function;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_code;
    uint8_t subclass;
    uint8_t prog_if;
    uint8_t header_type;
};

void pci_init(void);
size_t pci_device_count(void);
const struct pci_device* pci_device_get(size_t index);
const struct pci_device* pci_find_class(uint8_t class_code, uint8_t subclass);
