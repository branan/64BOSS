// These are very very simple printing functions. They don't know
// how to support any fancy characters except newlines, and they
// can't scroll. Use them only for important early information
// that can't wait for the actual console to be initialized.

#include "ktypes.h"

void earlycls();
void earlyprint(const char*);
void earlyprint(uint64);
void earlyprint(uint32);
void earlyprint(uint16);