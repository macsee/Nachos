// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "noff.h"

#define UserStackSize		1024 	// increase this as necessary!

class AddrSpace {
  public:
    AddrSpace(OpenFile *executable);	// Create an address space,
					// initializing it with the program
					// stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch

    int getVPage(int ppage);

    NoffHeader getNoffH () {
      return noffH;
    }

    OpenFile* getExec () {
      return exec;
    } 

    int getNumPages () {
      return numPages;
    }
#ifdef USE_TLB
    bool is_code(int i);
    bool is_data(int i);
    void demandLoading(int vpage, int ppage);
    TranslationEntry* getPage(int page);
	void setPhysPage (int vpage);
    void GetSwapFile(int pid);
    void SaveToSwap(int vpage);
    void GetFromSwap(int vpage);
    void UpdatepageTable (int vpage, int ppage);
    void PrintpageTable();
    //int getTLBentry();
    //int removePageFromTLB();
    TranslationEntry lastPageUsed;
    int lastTLBentry;
#endif
  private:
    TranslationEntry *pageTable;	// Assume linear page table translation
					// for now!
    unsigned int numPages;		// Number of pages in the virtual 
					// address space
    OpenFile* exec;
    NoffHeader noffH;
    #ifdef USE_TLB
        OpenFile* swap;
    #endif
};

#endif // ADDRSPACE_H
