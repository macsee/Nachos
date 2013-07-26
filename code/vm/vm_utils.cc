#ifdef USE_TLB
#include "vm_utils.h"

void pageFaultHandler(int virAddrReq) {

	stats->numTLBMiss++;
	stats->numMemAccess--; // Se debe restar el accesso a memoria ya que en un pagefault se re intenta leer esa dirccion de memoria hasta que se tiene éxito.
	int virPageReq = virAddrReq/PageSize;
    AddrSpace* owner_space;
    AddrSpace* currentSpace = currentThread->space;
    int phys_page = -1;
    int owner_vpage = -1;
    int page = -1;

    TranslationEntry* page_entry = currentSpace->getPage(virPageReq);
   
    // La pag no está en TLB. 
    // Debemos chequear si ya estaba cargada en memoria.

    if (page_entry->physicalPage < 0) {
        // Si no esta cargada en memoria, buscamos alguna pagina.
        phys_page = coreMap->GetPageLRUPerfecto();
        // phys_page = coreMap->GetPageFIFO();
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
        page_entry->physicalPage = phys_page;

        // Actualizamos el Coremap con los datos nuevos
        coreMap->Update(phys_page, virPageReq);

        // Chequeamos si la pagina solicitada estaba 
        // swapeada anteriormente.
        if (page_entry->valid) {
            // si no estaba swapeada cargamos normalmente
            currentSpace->demandLoading(virPageReq, phys_page);
        }
        else {
            // si estaba swapeada la traemos
            currentSpace->GetFromSwap(virPageReq);
        }
    } else {
        printf("*********************Llamo a incount con %d***************\n", page_entry->physicalPage);
        coreMap->IncCount(page_entry->physicalPage);
    }

    // Pedimos una pagina a la TLB si es que no se liberó en el swap y luego copiamos ahi.
    if (page < 0)
	   page = getTLBentry();

    machine->tlb[page] =  *page_entry; //currentSpace->getPage(virPageReq);//page_entry;    
   
    //DEBUG('f', "Virtual Page: %d\n", virPageReq);
    DEBUG('f', "Direccion virtual: %d\n", virAddrReq);
    DEBUG('f', "Pagina de la TLB: %d\n", page);
    DEBUG('f', "Physical page: %d\n", page_entry->physicalPage);//machine->tlb[page].physicalPage);
    DEBUG('f',"***************************************************************\n\n");           
    //currentSpace->PrintpageTable();
    //printf("\n");            
    //coreMap->PrintCoremap();
    //printf("\n");
}

int getTLBentry() 
{
    int page = -1;

    for (int i = 0; i < TLBSize; i++)
    {
        if (!machine->tlb[i].valid) {
            page = i;
            break;
        }    
    }

    if (page == -1) {
        page = rand() % TLBSize; // Política de elección de página de TLB
        currentThread->space->CopyTLBtoPageTable(page);  // Copiamos la page a la pagetable actualizada antes de sobreescribirla 
    }
        
    return page;           
}

void flushTLB()
{
	for (int i = 0; i < TLBSize; i++)
	{
		machine->tlb[i].valid = false;
	}
}

int getTLBindex(int ppage) {

    for (int i = 0; i < TLBSize; i++) {
        if (machine->tlb[i].physicalPage == ppage)
            return i;
    }
    return -1;     
}

void clearTLBEntry(int index) {

    machine->tlb[index].valid = false;
}


#endif
