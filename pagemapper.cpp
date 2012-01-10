#include "pagemapper.h"

#include "memallocator.h"
#include "pageallocator.h"
#include "kutils.h"

#include "earlyprint.h"

const uint64 pml4_self_entry      = 0x100;
const uint64 pml4_secondary_entry = 0x101;

uint64* pml4  = (uint64*)(0xffff000000000000 | (pml4_self_entry << 12) | (pml4_self_entry << 21) | (pml4_self_entry << 30) | (pml4_self_entry << 39));
uint64* pml4b = (uint64*)(0xffff000000000000 | (pml4_secondary_entry << 12) | (pml4_self_entry << 21) | (pml4_self_entry << 30) | (pml4_self_entry << 39));

inline uint64* pml3(uint64 addr) {
    uint64 result = 0xffff000000000000 | (pml4_self_entry << 21) | (pml4_self_entry << 30) | (pml4_self_entry << 39);
    result += (addr & 0x0000ff8000000000) >> 27;
    return (uint64*)result;
}

inline uint64* pml3b(uint64 addr) {
    uint64 result = 0xffff000000000000 | (pml4_secondary_entry << 21) | (pml4_self_entry << 30) | (pml4_self_entry << 39);
    result += (addr & 0x0000ff8000000000) >> 27;
    return (uint64*)result;
}

inline uint64* pml2(uint64 addr) {
    uint64 result = 0xffff000000000000 | (pml4_self_entry << 30) | (pml4_self_entry << 39);
    result += (addr & 0x0000ffffc0000000) >> 18;
    return (uint64*)result;
}

inline uint64* pml2b(uint64 addr) {
    uint64 result = 0xffff000000000000 | (pml4_secondary_entry << 30) | (pml4_self_entry << 39);
    result += (addr & 0x0000ffffc0000000) >> 18;
    return (uint64*)result;
}

inline uint64* pml1(uint64 addr) {
    uint64 result = 0xffff000000000000 | (pml4_self_entry << 39);
    result += (addr & 0x0000ffffffe00000) >> 9;
    return (uint64*)result;
}

inline uint64* pml1b(uint64 addr) {
    uint64 result = 0xffff000000000000 | (pml4_secondary_entry << 39);
    result += (addr & 0x0000ffffffe00000) >> 9;
    return (uint64*)result;
}

inline uint32 pml4_entry(uint64 addr) {
    return (addr & 0x0000ff8000000000) >> 39;
}

inline uint32 pml3_entry(uint64 addr) {
    return (addr & 0x0000007fc0000000) >> 30;
}

inline uint32 pml2_entry(uint64 addr) {
    return (addr & 0x000000003fe00000) >> 21;
}

inline uint32 pml1_entry(uint64 addr) {
    return (addr & 0x00000000001ff000) >> 12;
}

inline void invlpg(void* addr) {
    __asm__ volatile("invlpg %0"::"m" (*(char *)addr));
}

extern "C" void fixupStack();

void PageMapper::mapPages(uint64 start, uint64 count) {
    earlyprint("We got a request for pages... that should not be happening yet!");
}

const uint64 heap_addr = 0xffff810000000000;
const uint64 stack_addr = 0xffffffff7fffc000;

extern char bootheap_start;

// There are three ranges to build entries for:
// Heap, Stack, and Kernel
// 
// The   Heap is 16K at address 0xffff810000000000
// The  stack is 16K at address 0xFFFFFFFF7FFFC000
// The Kernel is ??K at address 0xFFFFFFFF80000000
// The kernel should already be mapped. We actually
// want to /UNMAP/ sections of the first 2MB that we
// aren't using.
void PageMapper::buildDefaultMapping() {
    uint64 heap_pml3 = PageAllocator::requestPage();
    pml4[0x102] = heap_pml3 | 0x03;
    invlpg(pml3(heap_addr));

    uint64 heap_pml2 = PageAllocator::requestPage();
    pml3(heap_addr)[0] = heap_pml2 | 0x03;
    invlpg(pml2(heap_addr));

    uint64 heap_pml1 = PageAllocator::requestPage();
    pml2(heap_addr)[0] = heap_pml1 | 0x03;
    invlpg(pml1(heap_addr));

    for(int i = 0; i < 4; i++) {
        pml1(heap_addr)[i] = PageAllocator::requestPage() | 0x03;
        invlpg((void*)(heap_addr + 0x1000 * i));
    }
    memcpy((void*)heap_addr, &bootheap_start, 0x4000);

    uint64 stack_pml2 = PageAllocator::requestPage();
    pml3(stack_addr)[pml3_entry(stack_addr)] = stack_pml2 | 0x03;
    invlpg(pml2(stack_addr));

    uint64 stack_pml1 = PageAllocator::requestPage();
    pml2(stack_addr)[pml2_entry(stack_addr)] = stack_pml1 | 0x03;
    invlpg(pml1(stack_addr));

    for(int i = 0; i < 4; i++) {
        uint64 addr = stack_addr + i * 0x1000;
        pml1(stack_addr)[pml1_entry(addr)] = PageAllocator::requestPage() | 0x03;
        invlpg((void*)addr);
    }

    MemAllocator::fixup();
    PageAllocator::fixup();
}

void PageMapper::removeBootPages() {
    // Now that the new heap and stack are ready, we can blow away
    // the old heap/stack and free those pages for other uses.
    for(int i = 0; i < 8; i++) {
        uint64 addr = (uint64)&bootheap_start + 0x1000*i;
        pml1(addr)[pml1_entry(addr)] = 0x0;
        invlpg((void*)addr);
    }
    PageAllocator::addPageRange((uint64)&bootheap_start-0xffffffff80000000, ((uint64)&bootheap_start)+0x8000-0xffffffff80000000);

    // The high mapping does not need the video memory and whatnot. Unmap the entire low megabyte from the kernel map.
    for(int i = 0; i < 256; i++) {
        pml1(0xFFFFFFFF80000000)[i] = 0;
        invlpg((void*)(0xFFFFFFFF80000000 + i*4096));
    }

    // TODO: figure out how to get rid of any extra mapped pages at the end of the kernel

    // Lastly, we create a new low mapping for video memory
    uint64 vid_pml2 = PageAllocator::requestPage();
    pml3(0xb8000)[pml3_entry(0xb8000)] = vid_pml2 | 0x03;
    invlpg(pml2(0xb8000));
    uint64 vid_pml1 = PageAllocator::requestPage();
    pml2(0xb8000)[pml2_entry(0xb8000)] = vid_pml1 | 0x03;
    invlpg(pml1(0xb8000));
    pml1(0xb8000)[pml1_entry(0xb8000)] = 0xb8003;
    invlpg((void*)0xb8000);

}


void* PageMapper::fixupHeap(const void* loc) {
    if(!loc)
        return 0;
    char* addr = (char*)loc;
    addr = (char*)(addr - &bootheap_start);
    addr += heap_addr;
    return addr;
}
