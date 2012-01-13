#include "modloader.h"
#include "interface/module.h"
#include "earlyprint.h"

ModuleLoader::ModuleLoader() {}

ModuleLoader* ModuleLoader::instance() {
    static ModuleLoader inst;
    return &inst;
}

bool ModuleLoader::loadModule(iModule* mod) {
}
