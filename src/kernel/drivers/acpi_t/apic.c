#include <drivers/acpi.h>
#include <stdint.h>
#include <stdbool.h>
#include <debug/debug.h>

void init_smp(void) {
    // Initialize the APIC and start Application Processors (APs)
    printk(GFX_WHITE, "Initializing SMP...\n");

    // Send INIT IPI to all APs
    send_ipi(0xFF, 0x4500);

    // Send STARTUP IPI to all APs
    send_ipi(0xFF, 0x4600 | (0x08 & 0xFF));

    printk(GFX_WHITE, "SMP initialization complete.\n");
}

void send_ipi(uint8_t apic_id, uint32_t ipi) {
    // Send Inter-Processor Interrupt (IPI) to the specified APIC ID
    uint32_t* icr = (uint32_t*)(0xFEE00000 + 0x300);
    icr[0] = (apic_id << 24) | ipi;
    icr[1] = 0;
}

uint32_t read_lapic(uintptr_t lapic_addr, uint64_t reg_offset) {
    uint32_t volatile *lapic_register_addr = (uint32_t volatile *) (((uint64_t)lapic_addr) + reg_offset);
    return (uint32_t) *lapic_register_addr;
}

void write_lapic(uintptr_t lapic_addr, uint64_t reg_offset, uint32_t val) {
    uint32_t volatile *lapic_register_addr = (uint32_t volatile *) (((uint64_t)lapic_addr) + reg_offset);
    *lapic_register_addr = val;
}

uint32_t read_ioapic(void *ioapic_addr, uint32_t reg) {
   uint32_t volatile *ioapic = (uint32_t volatile *)ioapic_addr;
   ioapic[0] = (reg & 0xff);
   return ioapic[4];
}

void write_ioapic(void *ioapic_addr, uint32_t reg, uint32_t value) {
   uint32_t volatile *ioapic = (uint32_t volatile *)ioapic_addr;
   ioapic[0] = (reg & 0xff);
   ioapic[4] = value;
}

void map_ioapic(uint8_t vec, uint32_t irq, uint32_t lapic_id, bool polarity, bool trigger) {
    uintptr_t ioapic_addr = (uintptr_t) (((uint64_t) kernel.ioapic_device.ioapic_addr) + kernel.hhdm);
    uint32_t gsi_base = kernel.ioapic_device.global_system_interrupt_base;
    uint32_t entry_num = gsi_base + (irq * 2);
    uint32_t reg_nums[2] = {0x10 + entry_num, 0x11 + entry_num};
    uint32_t redirection_entries[2] = {read_ioapic((void*) ioapic_addr, reg_nums[0]), read_ioapic((void*) ioapic_addr, reg_nums[1])};

    redirection_entries[0] = (redirection_entries[0] & ~0xFF) | vec; // set vector number
    redirection_entries[0] &= ~0x700; // set delivery mode to normal
    redirection_entries[0] &= ~0x800; // set destination mode to physical
    if (polarity)
        redirection_entries[0] |= 0x2000; // set polarity to low
    else
        redirection_entries[0] &= ~0x2000; // set polarity to high
    if (trigger)
        redirection_entries[0] |= 0x8000; // set trigger to level
    else
        redirection_entries[0] &= ~0x8000; // set trigger to edge
    redirection_entries[0] &= ~0x10000; // make sure that it's unmasked

    redirection_entries[1] = (lapic_id & 0xF) << 28;

    write_ioapic((void*) ioapic_addr, reg_nums[0], redirection_entries[0]);
    write_ioapic((void*) ioapic_addr, reg_nums[1], redirection_entries[1]);
}

void mask_ioapic(uint8_t irq, uint32_t lapic_id) {
    uintptr_t ioapic_addr = (uintptr_t) (((uint64_t) kernel.ioapic_device.ioapic_addr) + kernel.hhdm);
    uint32_t gsi_base = kernel.ioapic_device.global_system_interrupt_base;
    uint32_t entry_num = gsi_base + (irq * 2);
    uint32_t reg_num = 0x10 + entry_num;
    uint32_t redirection_entry = read_ioapic((void*) ioapic_addr, reg_num);
    redirection_entry |= 0x10000;
    write_ioapic((void*)ioapic_addr, reg_num, redirection_entry);
}

void unmask_ioapic(uint8_t irq, uint32_t lapic_id) {
    uintptr_t ioapic_addr = (uintptr_t) (((uint64_t) kernel.ioapic_device.ioapic_addr) + kernel.hhdm);
    uint32_t gsi_base = kernel.ioapic_device.global_system_interrupt_base;
    uint32_t entry_num = gsi_base + (irq * 2);
    uint32_t reg_num = 0x10 + entry_num;
    uint32_t redirection_entry = read_ioapic((void*) ioapic_addr, reg_num);
    redirection_entry &= ~0x10000;
    write_ioapic((void*)ioapic_addr, reg_num, redirection_entry);
}

void init_local_apic(uintptr_t lapic_addr) {
    write_lapic(lapic_addr, LAPIC_TASK_PRIORITY_REGISTER, 0);
    write_lapic(lapic_addr, LAPIC_DESTINATION_FORMAT_REGISTER, 0xF0000000);
    write_lapic(lapic_addr, LAPIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER, 0xFF | 0x100);
}

void init_apic() {
    if (!verify_apic()) {
        panic("This device does not support APIC, halting.\n");
    }

    // Disable PIC
    outb(0x21, 0xff);
    outb(0xA1, 0xff);

    MADT *madt = (MADT*) find_MADT(kernel.rsdt);
    map_pages((uint64_t*) (kernel.cr3 + kernel.hhdm), (uint64_t) madt->local_apic_addr + kernel.hhdm, (uint64_t) madt->local_apic_addr, 1, KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE);
    uint64_t lapic_registers_virt = (uint64_t) madt->local_apic_addr + kernel.hhdm;
    kernel.lapic_addr = lapic_registers_virt;
    init_local_apic(lapic_registers_virt);

    MADTEntryHeader *entry = (MADTEntryHeader*) (((uint64_t) madt) + sizeof(MADT));
    uint64_t incremented = sizeof(MADT);
    while (incremented < madt->header.length) {
        if (entry->entry_type == IOAPIC) {
            IOApic *this_ioapic = (IOApic*) entry;
            map_pages((uint64_t*) (kernel.cr3 + kernel.hhdm), (uint64_t) this_ioapic->ioapic_addr + kernel.hhdm, (uint64_t) this_ioapic->ioapic_addr, 1, KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE);
            kernel.ioapic_device = *this_ioapic;
            kernel.ioapic_addr   = this_ioapic->ioapic_addr;
        } else if (entry->entry_type == LOCAL_APIC) {
            ProcessorLocalAPIC *this_local_apic = (ProcessorLocalAPIC*) entry;
        }
        entry = (MADTEntryHeader*) (((uint64_t) entry) + entry->record_length);
        incremented += entry->record_length;
    }
}


