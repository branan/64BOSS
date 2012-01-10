#ifndef PAGEMAPPER_H
#define PAGEMAPPER_H

#include "ktypes.h"

namespace PageMapper {
    void mapPages(uint64, uint64);
    void unmapPages(uint64, uint64);

    // returns the physical address of the new PML4
    uint64 forkMapping();
    void buildDefaultMapping();
    void removeBootPages();

    void* fixupHeap(const void*);
};

#endif // PAGEMAPPER_H
