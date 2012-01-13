#ifndef MODULES_MODULE_H
#define MODULES_MODULE_H

#ifdef MODULE_BUILTIN
#include "kernel.h"
#include "modloader.h"
#define KMODULE(ClassName) struct ClassName##Creator { ClassName##Creator(){ ModuleLoader::instance()->loadModule(new ClassName(Kernel::instance())); } } ClassName##_creator;
#else
#define KMODULE(ClassName) extern "C" void* create_module() { return new ClassName; }
#endif

#endif // MODULES_MODULE_H

