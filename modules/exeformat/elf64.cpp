#include "module.h"
#include "earlyprint.h"
#include "interface/kernel.h"
#include "interface/module.h"

class ElfModule : public iModule {
public:
    ElfModule(iKernel*) {
    }

    const char* name() const {
        return "ELF64";
    }
};

KMODULE(ElfModule)
