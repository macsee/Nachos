#ifndef VM_UITLS_H
#define VM_UTILS_H

#include "system.h"

void pageFaultHandler(int vpage);
int getTLBentry();
void flushTLB();
int getTLBindex(int ppage);
void clearTLBEntry(int index);

#endif // VM_UTILS_H