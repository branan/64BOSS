#ifndef INTERFACE_MODULE_H
#define INTERFACE_MODULE_H

class iModule {
public:
    // An internal name that uniquely
    // identifies the module.
    virtual const char* name() const =0;
};

#endif // INTERFACE_MODULE_H
