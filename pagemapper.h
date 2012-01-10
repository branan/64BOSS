#ifndef PAGEMAPPER_H
#define PAGEMAPPER_H

#include "ktypes.h"

class PageMapper {
public:
    static void mapPages(uint64, uint64);
    static void unmapPages(uint64, uint64);

    // returns the physical address of the new PML4
    static uint64 forkMapping();
    static void buildDefaultMapping();
    static void removeBootPages();

    static void* fixupHeap(const void*);
};

#endif // PAGEMAPPER_H
