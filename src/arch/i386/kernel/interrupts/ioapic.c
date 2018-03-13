#include <main.h>
#include <kprint.h>
#include <io.h>
#include <arch/i386/kernel/ioapic.h>
#include <arch/i386/kernel/interrupts.h>
#include <kernel/paging.h>

// https://wiki.osdev.org/IOAPIC

static void ioapic_write(uint8_t offset, uint32_t value) {
    // Fill the I/O APIC's register selection memory area with our requested register offset.
    *(volatile uint32_t*)(IOAPIC_ADDRESS + IOAPIC_IOREGSL) = offset;

    // Write data into the I/O APIC's data window memory area.
    *(volatile uint32_t*)(IOAPIC_ADDRESS + IOAPIC_IOWIN) = value;
}

static uint32_t ioapic_read(uint8_t offset) {
    // Fill the I/O APIC's register selection memory area with our requested register offset.
    *(volatile uint32_t*)(IOAPIC_ADDRESS + IOAPIC_IOREGSL) = offset;

    // Read the result from the I/O APIC's data window memory area.
    return *(volatile uint32_t*)(IOAPIC_ADDRESS + IOAPIC_IOWIN);
}

uint8_t ioapic_id() {
    return (uint8_t)((ioapic_read(IOAPIC_REG_ID) >> 24) & 0xF);
}

uint8_t ioapic_version() {
    return (uint8_t)(ioapic_read(IOAPIC_REG_VERSION) & 0xFF);
}

uint8_t ioapic_max_interrupts() {
    return (uint8_t)(((ioapic_read(IOAPIC_REG_VERSION) >> 16) & 0xFF) + 1);
}

ioapic_redirection_entry_t ioapic_get_redirection_entry(uint8_t interrupt) {
    // Determine register offset.
    uint8_t offset = IOAPIC_REG_REDTBL + (interrupt * 2);

    // Get entry from I/O APIC.
    uint64_t data = (uint64_t)ioapic_read(offset) | ((uint64_t)ioapic_read(offset + 1) << 32);
    return *(ioapic_redirection_entry_t*)&data;
}

void ioapic_set_redirection_entry(uint8_t interrupt, ioapic_redirection_entry_t entry) {
    // Determine register offset and get pointer to entry.
    uint8_t offset = IOAPIC_REG_REDTBL + (interrupt * 2);
    uint32_t *data = (uint32_t*)&entry;
    
    // Send data to I/O APIC.
    ioapic_write(offset, data[0]);
    ioapic_write(offset + 1, data[1]);
}

void ioapic_enable_interrupt(uint8_t interrupt, uint8_t vector) {
    // Get entry for interrupt.
    ioapic_redirection_entry_t entry = ioapic_get_redirection_entry(interrupt);

    // Set entry fields.
    entry.interruptVector = vector;
    entry.deliveryMode = IOAPIC_DELIVERY_FIXED;
    entry.destinationMode = IOAPIC_DEST_MODE_PHYSICAL;
    entry.interruptMask = false;
    entry.destinationField = 0;

    // Save entry to I/O APIC.
    ioapic_set_redirection_entry(interrupt, entry);
    kprintf("IOAPIC: Mapped interrupt %u to 0x%X\n", interrupt, vector);
}

void ioapic_disable_interrupt(uint8_t interrupt) {
    // Get entry for interrupt and mask it.
    ioapic_redirection_entry_t entry = ioapic_get_redirection_entry(interrupt);
    entry.interruptMask = true;

    // Save entry to I/O APIC.
    ioapic_set_redirection_entry(interrupt, entry);
}

void ioapic_init(uintptr_t base) {
    kprintf("IOAPIC: Initializing I/O APIC at 0x%X...\n", base);

    // Map I/O APIC to virtual memory.
    paging_map_virtual_to_phys(IOAPIC_ADDRESS, base);

    // Get info about I/O APIC.
    uint8_t maxInterrupts = ioapic_max_interrupts();
    kprintf("IOAPIC: Mapped I/O APIC to 0x%X!\n", IOAPIC_ADDRESS);
    kprintf("IOAPIC: ID: %u\n", ioapic_id());
    kprintf("IOAPIC: Version: 0x%X.\n", ioapic_version());
    kprintf("IOPAIC: Max interrupts: %u\n", maxInterrupts);

    // Disable all interrupts.
    for (uint8_t i = 0; i < maxInterrupts; i++)
        ioapic_disable_interrupt(i);
    kprintf("I/O APIC initialized!\n");
}