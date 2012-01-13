#ifndef MODLOADER_H
#define MODLOADER_H

#include "ktypes.h"

class iModule;

class ModuleLoader {
public:
    // "Load" a module from the given physical memory range
    bool loadModule(uint64, uint64);

    // "Load" a module by a pointer to its handle
    // If this module is unloaded, the instance will be deleted but
    // the module data won't be removed.
    bool loadModule(iModule*);

    // Load a module from the given filename
    bool loadModule(const char*);

    // Although builtin modules can be unloaded, doing so
    // simply destroys the module object. the memory is not reclaimed.
    bool unloadModule(const char*);

    static ModuleLoader* instance();
private:
    ModuleLoader();
    // kmap<kstring,modref> loaded_modules;
};

#endif // MODLOADER_H
