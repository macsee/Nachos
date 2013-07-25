// addrspace.cc 
//  Routines to manage address spaces (executing user programs).
//
//  In order to run a user program, you must:
//
//  1. link with the -N -T 0 option 
//  2. run coff2noff to convert the object file to Nachos format
//      (Nachos object code format is essentially just a simpler
//      version of the UNIX executable object code format)
//  3. load the NOFF file into the Nachos file system
//      (if you haven't implemented the file system yet, you
//      don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "userfunctions.h"
#ifdef USE_TLB
    #include "vm_utils.h"
#endif    

//----------------------------------------------------------------------
// SwapHeader
//  Do little endian to big endian conversion on the bytes in the 
//  object file header, in case the file was generated on a little
//  endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
    noffH->noffMagic = WordToHost(noffH->noffMagic);
    noffH->code.size = WordToHost(noffH->code.size);
    noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
    noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
    noffH->initData.size = WordToHost(noffH->initData.size);
    noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
    noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
    noffH->uninitData.size = WordToHost(noffH->uninitData.size);
    noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
//  Create an address space to run a user program.
//  Load the program from a file "executable", and set everything
//  up so that we can start executing user instructions.
//
//  Assumes that the object code file is in NOFF format.
//
//  First, set up the translation from program memory to physical 
//  memory.  For now, this is really simple (1:1), since we are
//  only uniprogramming, and we have a single unsegmented page table
//
//  "executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable)
{
    unsigned int i, size;

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
        (WordToHost(noffH.noffMagic) == NOFFMAGIC))
        SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

    exec = executable;

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
            + UserStackSize;    // we need to increase the size
                        // to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;
    
#ifndef USE_TLB
    ASSERT(numPages <= listPages->NumClear());
#endif
    
    //ASSERT(numPages <= NumPhysPages);     // check we're not trying
                        // to run anything too big --
                        // at least until we have
                        // virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
                    numPages, size);

    // then, copy in the code and data segments into memory
    
// first, set up the translation 
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++) {
    pageTable[i].virtualPage = i;   // for now, virtual page # = phys page #
#ifdef USE_TLB
    pageTable[i].physicalPage = -1; //listPages->Find();
#else
    pageTable[i].physicalPage = listPages->Find();
#endif
    pageTable[i].valid = true;
    pageTable[i].use = false;
    pageTable[i].dirty = false;
    pageTable[i].readOnly = false;  // if the code segment was entirely on 
                    // a separate page, we could set its 
                    // pages to be read-only
#ifndef USE_TLB   
    bzero(&machine->mainMemory[pageTable[i].physicalPage * PageSize], PageSize);
#endif
}
    
// zero out the entire address space, to zero the unitialized data segment 
// and the stack segment

    
/*
// then, copy in the code and data segments into memory
    if (noffH.code.size > 0) {
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", 
            noffH.code.virtualAddr, noffH.code.size);
        executable->ReadAt(&(machine->mainMemory[noffH.code.virtualAddr]),
            noffH.code.size, noffH.code.inFileAddr); 
    }
    if (noffH.initData.size > 0) {
        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", 
            noffH.initData.virtualAddr, noffH.initData.size);
        executable->ReadAt(&(machine->mainMemory[noffH.initData.virtualAddr]),
            noffH.initData.size, noffH.initData.inFileAddr);
    }*/
    #ifdef USE_TLB
        swap = NULL;
    #endif
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//  Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    for (int i = 0; i < numPages; ++i)
    {
		if (pageTable[i].physicalPage >= 0) {
#ifdef USE_TLB
            coreMap->Clear(pageTable[i].physicalPage);
            //delete swapfile;
#else
            listPages->Clear(pageTable[i].physicalPage);
#endif  
        }	
    }
    delete pageTable;
    delete exec; // Agregado para borrar el executable
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
//  Set the initial values for the user-level register set.
//
//  We write these directly into the "machine" registers, so
//  that we can immediately jump to user code.  Note that these
//  will be saved/restored into the currentThread->userRegisters
//  when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
    machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);   

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
//  On a context switch, save any machine state, specific
//  to this address space, that needs saving.
//
//  For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
//  On a context switch, restore the machine state so that
//  this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void 
AddrSpace::RestoreState() 
{
#ifdef USE_TLB
    for (int i = 0; i < TLBSize; i++)
        machine->tlb[i].valid = false;
#else
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
#endif
}


#ifdef USE_TLB

    //GetSwapFile se llama desde getPid
void AddrSpace::GetSwapFile(int pid){
    if (swap == NULL)
    {
        swapfile = new char[128];
        sprintf (swapfile, "swap-%d.asid", pid);
        fileSystem->Create(swapfile, numPages * PageSize); //Creo archivo SWAP     
        swap = fileSystem->Open(swapfile); //devuelvo el puntero al archivo abierto
    }
}


void AddrSpace::SaveToSwap(int vpage){

    DEBUG('k', "<<======== Swapping in virtual page %d from physical page %d\n", vpage, pageTable[vpage].physicalPage);

    //Asigno valores a la pageTable del thread saliente

    char page[PageSize];
    for (int i = 0; i < PageSize; i++)
    {
        page[i] = machine->mainMemory[pageTable[vpage].physicalPage * PageSize + i];
        //printf("Value: %d\n", pageTable[vpage].physicalPage * PageSize + i);
    }
    swap->WriteAt(page, PageSize, vpage*PageSize);

    pageTable[vpage].valid = false;
    pageTable[vpage].physicalPage = -1;
}

void AddrSpace::GetFromSwap(int vpage){
    DEBUG('k', "========>> Swapping back virtual page %d into physical page %d\n",vpage, pageTable[vpage].physicalPage);

    char page[PageSize];
    swap->ReadAt(page, PageSize, vpage*PageSize);
    for (int i = 0; i < PageSize; i++)
    {
        machine->mainMemory[pageTable[vpage].physicalPage * PageSize + i] = page[i];
        //printf("Value: %d\n", pageTable[vpage].physicalPage * PageSize + i);
    }
    pageTable[vpage].valid = true;
}

TranslationEntry* 
AddrSpace::getPage(int vpage) {
    return &pageTable[vpage];
}

int AddrSpace::setPhysPage (int vpage) { 
    // pageTable[vpage].physicalPage = listPages->Find();

    stats->numTLBMiss++;
    stats->numMemAccess--; // Se debe restar el accesso a memoria ya que en un pagefault se re intenta leer esa dirccion de memoria hasta que se tiene éxito.
    AddrSpace* owner_space;
    //AddrSpace* currentSpace = currentThread->space;
    int phys_page = -1;
    int owner_vpage = -1;
    int page = -1;

    //TranslationEntry* page_entry = pageTable[vpage];
   
    // La pag no está en TLB. 
    // Debemos chequear si ya estaba cargada en memoria.

        // Si no esta cargada en memoria, buscamos alguna pagina.
        // phys_page = coreMap->GetPageFIFO();
        phys_page = coreMap->GetPageLRU();
        DEBUG('k', "Physical page %d selected by algorithm!\n",phys_page);
        // Si la phys_page esta libre no hacemos nada.
        // De lo contrario tenemos que swapear.
        if (!coreMap->IsFree(phys_page)) {
            owner_space = coreMap->GetOwner(phys_page);
            owner_vpage = coreMap->GetVpage(phys_page);

            // chequeamos si la phys_page estaba en la TLB
            // y si es así la marcamos para quitar, pero antes la copiamos.
            page = getTLBindex(phys_page);
            if (page >= 0)
                owner_space->CopyTLBtoPageTable(page);

            owner_space->SaveToSwap(owner_vpage); // Guardamos en SWAP
        
        }
        else
            DEBUG('k', "No swapping needed!\n");

        // Actualizamos la entrada de la
        // pagetable con la physpage nueva
        pageTable[vpage].physicalPage = phys_page;
        //page_entry->physicalPage = phys_page;

        // Actualizamos el Coremap con los datos nuevos
        coreMap->Update(phys_page, vpage);

        return phys_page;
}

bool AddrSpace::is_code (int i) {
    return (i >= noffH.code.virtualAddr && i < noffH.code.virtualAddr + noffH.code.size);
}

bool AddrSpace::is_data (int i) {
    return (i >= noffH.initData.virtualAddr && i < noffH.initData.virtualAddr + noffH.initData.size);
}

void AddrSpace::demandLoading(int vpage, int ppage) 
{
    DEBUG('f', "Virtual page %d demanded. To be loaded in physical page %d\n\n", vpage, ppage);
    char byte;

    for (int i = vpage*PageSize; i < (vpage+1)*PageSize; i++)
    {
        if (is_code(i)) {
            exec->ReadAt(&byte,1,noffH.code.inFileAddr + i - noffH.code.virtualAddr);
            //printf("Byte: %c - i: %d\n",byte, i );
		   machine->mainMemory[ppage*PageSize+i-vpage*PageSize] = byte; 
           //machine->WriteMem(i,1,byte);
        }
        else if (is_data(i)) {
            exec->ReadAt(&byte,1,noffH.initData.inFileAddr + i - noffH.initData.virtualAddr);
   		   machine->mainMemory[ppage*PageSize+i-vpage*PageSize] = byte; 

 //machine->WriteMem(i,1,byte);
        }
        else {
		   machine->mainMemory[ppage*PageSize+i-vpage*PageSize] = 0; 
//            machine->WriteMem(i,1,0);
        }
    }
}

void AddrSpace::UpdatepageTable(int vpage, int ppage) {
    pageTable[vpage].physicalPage = ppage;
}

void AddrSpace::PrintpageTable() {
    for (int i = 0; i < numPages; i++) {
        printf("PageTable[%d] = {PhysicalPage %d | Valid %d}\n", i, pageTable[i].physicalPage, pageTable[i].valid);
    }
}

void AddrSpace::CopyTLBtoPageTable(int index) {
    TranslationEntry entry = machine->tlb[index];
    pageTable[entry.virtualPage] = entry; 
}
#endif

//TranslationEntry AddrSpace::getPage(int page) { return pageTable[page]; }
