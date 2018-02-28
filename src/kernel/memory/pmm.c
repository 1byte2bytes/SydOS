#include <main.h>
#include <kprint.h>
#include <multiboot.h>
#include <string.h>
#include <math.h>
#include <kernel/pmm.h>

// Kernel's starting and ending addresses in RAM.
extern uint32_t KERNEL_VIRTUAL_START;
extern uint32_t KERNEL_VIRTUAL_END;
extern uint32_t KERNEL_VIRTUAL_OFFSET;
extern uint32_t PAGE_STACK_START;
extern uint32_t PAGE_STACK_END;

// Used to store info about memory in the system.
mem_info_t memInfo;

// Page stack, stores addresses to 4K pages in physical memory.
static page_t *pageStack;
static uint32_t pagesAvailable = 0;

// Pushes a page to the stack.
void pmm_push_page(page_t page) {
    // Increment stack pointer and check its within bounds.
    pageStack++;
    if (((uint32_t)(pageStack)) < memInfo.pageStackStart || ((uint32_t)(pageStack)) >= memInfo.pageStackEnd)
        panic("Page stack pointer out of bounds!\n");

    // Push page to stack.
    *pageStack = page;
    pagesAvailable++;
}

// Pops a page off the stack.
page_t pmm_pop_page() {
    // Get page off stack.
    page_t page = *pageStack;

    // Verify there are pages.
    if (pagesAvailable == 0)
        panic("No more pages!\n");

    // Decrement stack and return page.
    pageStack--;
    pagesAvailable--;
    return page;
}

// Builds the stack.
static void pmm_build_stack() {
    // Initialize stack.
    kprintf("Initializing page stack at 0x%X...\n", memInfo.pageStackStart);
    pageStack = (page_t*)(memInfo.pageStackStart);
    memset(pageStack, 0, memInfo.pageStackEnd - memInfo.pageStackStart);

    // Perform memory test on stack areas.
	kprintf("Testing %uKB of memory at 0x%X...\n", (memInfo.pageStackEnd - memInfo.pageStackStart) / 1024, memInfo.pageStackStart);
	for (page_t i = 0; i <= (memInfo.pageStackEnd - memInfo.pageStackStart) / sizeof(page_t); i++)
		pageStack[i] = i;

    bool pass = true;
	for (page_t i = 0; i <= (memInfo.pageStackEnd - memInfo.pageStackStart) / sizeof(page_t); i++)
		if (pageStack[i] != i) {
			pass = false;
			break;
		}
	kprintf("Test %s!\n", pass ? "passed" : "failed");
    if (!pass)
        panic("Memory test of page stack area failed.\n");

    // Build stack of free pages.
	for (multiboot_memory_map_t *entry = memInfo.mmap; (uint32_t)entry < (uint32_t)memInfo.mmap + memInfo.mmapLength;
		entry = (multiboot_memory_map_t*)((uint32_t)entry + entry->size + sizeof(entry->size))) {
		
		// If not available memory or 64-bit address (greater than 4GB), skip over.
		if (entry->type != MULTIBOOT_MEMORY_AVAILABLE || entry->addr & 0xFFFFFFFF00000000)
			continue;

        // Add pages to stack.
        uint32_t pageBase = ALIGN_4K(entry->addr);	
        kprintf("Adding pages in 0x%X!\n", pageBase);			
		for (uint32_t i = 0; i < entry->len / PAGE_SIZE_4K; i++) {
			uint32_t addr = pageBase + (i * PAGE_SIZE_4K);

			// If the address is in conventional memory (low memory), or is reserved by
			// the kernel, Multiboot header, or the page stack, don't mark it free.
			if (addr <= 0x100000 ||
                (addr >= (memInfo.kernelStart - memInfo.kernelVirtualOffset) && addr <= (memInfo.kernelEnd - memInfo.kernelVirtualOffset)) || 
                (addr >= (memInfo.pageStackStart - memInfo.kernelVirtualOffset) && addr <= (memInfo.pageStackEnd - memInfo.kernelVirtualOffset)) ||
                addr >= entry->addr + entry->len)
				continue;

            // Add page to stack.
            pmm_push_page(addr);
		}       
	}

    kprintf("Added %u pages!\n", pagesAvailable);
}

// Initializes the physical memory manager.
void pmm_init(multiboot_info_t* mbootInfo) {
	// Store away Multiboot info.
	memInfo.mbootInfo = mbootInfo;
	memInfo.mmap = (multiboot_memory_map_t*)mbootInfo->mmap_addr;
	memInfo.mmapLength = mbootInfo->mmap_length;

	// Store where the Multiboot info structure actually resides in memory.
	memInfo.mbootStart = (uint32_t)mbootInfo;
	memInfo.mbootEnd = (uint32_t)(mbootInfo + sizeof(multiboot_info_t));

	// Store where the kernel is. These come from the linker.
    memInfo.kernelVirtualOffset = (uint32_t)&KERNEL_VIRTUAL_OFFSET;
	memInfo.kernelStart = (uint32_t)&KERNEL_VIRTUAL_START;
	memInfo.kernelEnd = (uint32_t)&KERNEL_VIRTUAL_END;

    // Store page stack location. This is determined during early boot in kernel_main_early().
    memInfo.pageStackStart = PAGE_STACK_START;
    memInfo.pageStackEnd = PAGE_STACK_END;

	kprintf("Physical memory map:\n");
	uint64_t memory = 0;

	uint32_t base = mbootInfo->mmap_addr;
	uint32_t end = base + mbootInfo->mmap_length;
	multiboot_memory_map_t* entry = (multiboot_memory_map_t*)base;

	for(; base < end; base += entry->size + sizeof(uint32_t)) {
		entry = (multiboot_memory_map_t*)base;

		// Print out info.
		kprintf("region start: 0x%llX length: 0x%llX type: 0x%X\n", entry->addr, entry->len, (uint64_t)entry->type);

		if(entry->type == 1 && ((uint32_t)entry->addr) > 0)
			memory += entry->len;
	}

	// Print summary.
	kprintf("Kernel start: 0x%X | Kernel end: 0x%X\n", memInfo.kernelStart, memInfo.kernelEnd);
	kprintf("Multiboot info start: 0x%X | Multiboot info end: 0x%X\n", memInfo.mbootStart, memInfo.mbootEnd);
    kprintf("Page stack start: 0x%X | Page stack end: 0x%X\n", memInfo.pageStackStart, memInfo.pageStackEnd);

    // Build stack.
    pmm_build_stack();
    
    memInfo.memoryKb = memory / 1024;
	kprintf("Detected usable RAM: %uKB\n", memInfo.memoryKb);
    kprintf("Physical memory manager initialized!\n");
}
