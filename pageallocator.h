#ifndef PAGEALLOCATOR_H
#define PAGEALLOCATOR_H

#include "ktypes.h"

namespace PageAllocator {
    uint64 requestPage();
    uint64 requestBigPage();

    void releasePage(uint64);
    void releaseBigPage(uint64);

    // Do NOT call these functions unless you absolutely know
    // what you're doing. Add bad pages and the entire world crashes down.
    void addPageRange(uint64 start, uint64 end);
    void reservePageRange(uint64 start, uint64 end);

    // This should be called once the runtime heap has been allocated at its proper location
    void fixup();

    void printDebugInfo();

    void init();
};

#endif // PAGEALLOCATOR_H
