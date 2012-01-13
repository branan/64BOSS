// This file defines various functions required for C++ support

// Do I actually need to define this thing this way in long mode?
__extension__ typedef int __guard __attribute__((mode(__DI__)));

extern "C" int __cxa_guard_acquire(__guard* g) {
    //TODO: acquire a lock
    return !*(char *)(g); 
}

extern "C" void __cxa_guard_release(__guard* g) {
    //TODO: release a lock
    *(char *)g = 1;
}

extern "C" void __cxa_pure_virtual() {
    //TODO: kernel panic
}
