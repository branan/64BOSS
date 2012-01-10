#ifndef KUTILS_H
#define KUTILS_H

#include "ktypes.h"

void memcpy(void* dst, const void* src, uint64 sz);
void memset(void* dst, uint8 val, uint64 size);
void memset(void* dst, uint16 val, uint64 size);
void memset(void* dst, uint32 val, uint64 size);
void memset(void* dst, uint64 val, uint64 size);
char* strdup(const char* src);
uint64 strlen(const char* src);

void khalt();

#endif // KUTILS_H