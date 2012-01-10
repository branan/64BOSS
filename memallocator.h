#ifndef MEMALLOCATOR_H
#define MEMALLOCATOR_H

#include "ktypes.h"

class MemAllocator {
public:
    struct FreeEntry;

    void* alloc(uint64);
    void dealloc(void*);

    // This should be called once the runtime heap has been allocated at its final location
    void fixup();

    MemAllocator();
private:
    FreeEntry *free_list;
    uint64 heap_end;
};

extern MemAllocator mem_allocator;

#endif // MEMALLOCATOR_H