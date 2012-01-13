#ifndef KERNEL_H
#define KERNEL_H

#include "interface/kernel.h"

// This class is the implementation of the common interface between modules and the kernel.
// It is initialized the first time a module is loaded via a static constructor.
// If you are a module, you should include interface/kernel.h instead.

class Kernel : public iKernel {
public:
    static Kernel* instance();
private:
    Kernel();
};

#endif // KERNEL_H
