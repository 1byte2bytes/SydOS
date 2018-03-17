#include <main.h>
#include <tools.h>
#include <kprint.h>
#include <string.h>
#include <kernel/memory/paging.h>

#include <kernel/interrupts/interrupts.h>
#include <kernel/memory/pmm.h>

// http://www.rohitab.com/discuss/topic/31139-tutorial-paging-memory-mapping-with-a-recursive-page-directory/
// https://forum.osdev.org/viewtopic.php?f=15&t=19387
// https://medium.com/@connorstack/recursive-page-tables-ad1e03b20a85

void paging_change_directory(uintptr_t directoryPhysicalAddr) {
    // Tell CPU the directory and enable paging.
    asm volatile ("mov %%eax, %%cr3": :"a"(directoryPhysicalAddr)); 
    asm volatile ("mov %cr0, %eax");
    asm volatile ("orl $0x80000000, %eax");
    asm volatile ("mov %eax, %cr0");
}

void paging_flush_tlb() {
    // Flush TLB.
    asm volatile ("mov %cr3, %eax");
    asm volatile ("mov %eax, %cr3");
}

void paging_flush_tlb_address(uintptr_t address) {
    // Flush specified address in TLB.
    asm volatile ("invlpg (%0)" : : "b"(address) : "memory");
}

/*void paging_map_region(page_t *directory, page_t startAddress, page_t endAddress, bool kernel, bool writeable) {
    // Ensure addresses are on 4KB boundaries.
    startAddress = MASK_PAGE_4K(startAddress);
    endAddress = MASK_PAGE_4K(endAddress);

    // Map space.
    
}*/

static void paging_pagefault_handler(registers_t *regs) {
    page_t addr;
    asm volatile ("mov %%cr2, %0" : "=r"(addr));
    //kprintf("EAX: 0x%X, EBX: 0x%X, ECX: 0x%X, EDX: 0x%X\n", regs->eax, regs->ebx, regs->ecx, regs->edx);
    //kprintf("ESI: 0x%X, EDI: 0x%X, EBP: 0x%X, ESP: 0x%X\n", regs->esi, regs->edi, regs->ebp, regs->esp);
    //kprintf("EIP: 0x%X, EFLAGS: 0x%X\n", regs->eip, regs->eflags);
    panic("Page fault at 0x%X!\n", addr);
}

void paging_init() {
    kprintf("PAGING: Initializing...\n");

    // Wire up page fault handler.
    interrupts_isr_install_handler(ISR_EXCEPTION_PAGE_FAULT, paging_pagefault_handler);

#ifdef X86_64
    // Setup 4-level (long mode) paging.
    paging_late_long();
#else
    // Is PAE enabled?
    if (memInfo.paeEnabled)
        paging_late_pae(); // Use PAE paging.
    else 
        paging_late_std(); // No PAE, using standard paging.
#endif
        
    // Change to use our new page directory.
    paging_change_directory(memInfo.kernelPageDirectory);

    // Pop physical page for test.
    page_t page = pmm_pop_frame();
    kprintf("Popped page 0x%X for test...\n", page);
    
    // Map physical page to 0x1000 for testing.
    paging_map_virtual_to_phys(0x1000, page);

    // Test memory at location.
    kprintf("Testing memory at virtual address 0x1000...\n");
    uint32_t *testPage = (uint32_t*)0x1000;
    for (uint32_t i = 0; i < PAGE_SIZE_4K / sizeof(uint32_t); i++)
        testPage[i] = i;

    bool pass = true;
    for (uint32_t i = 0; i < PAGE_SIZE_4K / sizeof(uint32_t); i++)
        if (testPage[i] != i) {
            pass = false;
            break;
        }
    kprintf("Test %s!\n", pass ? "passed" : "failed");
    if (!pass)
        panic("Memory test of virtual address 0x1000 failed.\n");

    // Unmap virtual address and return page to stack.
    kprintf("Unmapping 0x1000 and pushing page 0x%X back to stack...\n", page);
    paging_unmap_virtual(0x1000);
    pmm_push_frame(page);

    kprintf("PAGING: Initialized!\n");
}
