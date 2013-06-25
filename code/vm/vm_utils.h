#ifndef VM_UITLS_H
#define VM_UTILS_H

#include "system.h"

void pageFaultHandler(int vpage);
int getTLBentry();
void flushTLB();

#endif // VM_UTILS_H