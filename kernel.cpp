#include "kernel.h"
#include "earlyprint.h"

Kernel::Kernel() {}

Kernel* Kernel::instance() {
    static Kernel inst;
    return &inst;
}
