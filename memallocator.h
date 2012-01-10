#ifndef MEMALLOCATOR_H
#define MEMALLOCATOR_H

#include "ktypes.h"

namespace MemAllocator {
    void* alloc(uint64);
    void dealloc(void*);

    // This should be called once the runtime heap has been allocated at its final location
    void fixup();

    void init();
};

#endif // MEMALLOCATOR_H
