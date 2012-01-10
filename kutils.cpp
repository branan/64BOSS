#include "kutils.h"

uint64 strlen(const char* src) {
    const char* start = src;
    while(*src++);
    return src - start;
}

char* strdup(const char* src) {
    if(!src)
        return 0;
    uint64 len = strlen(src);
    char* dst = new char[len];
    memcpy(dst, src, len);
    return dst;
}