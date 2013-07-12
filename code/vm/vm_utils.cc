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

    TranslationEntry* page_entry = currentSpace->getPage(virPageReq);
   
    // La pag no está en TLB. 
    // Debemos chequear si ya estaba cargada en memoria.

    if (page_entry->physicalPage < 0) {
        // Si nunca se cargó buscamos una pagina en memoria.
        phys_page = coreMap->GetPageLRU();
        DEBUG('k', "Physical page %d selected by algorithm!\n",phys_page);
        // Si la phys_page esta libre no hacemos nada.
        // De lo contrario tenemos que swapear.
        if (!coreMap->IsFree(phys_page)) {
            owner_space = coreMap->GetOwner(phys_page);
            owner_vpage = coreMap->GetVpage(phys_page);
            owner_space->SaveToSwap(owner_vpage); // Guardamos en SWAP
        
            // chequeamos si la phys_page estaba en la TLB
            // y si es así la quitamos.
            int index = getTLBindex(phys_page);
            if (index >= 0)
                clearTLBEntry(index);
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
    }

    // Pedimos una pagina a la TLB y luego copiamos ahi.
	int page = getTLBentry();
    machine->tlb[page] =  *page_entry; //currentSpace->getPage(virPageReq);//page_entry;    
   
    //DEBUG('f', "Virtual Page: %d\n", virPageReq);
    DEBUG('f', "Direccion virtual: %d\n", virAddrReq);
    DEBUG('f', "Pagina de la TLB: %d\n", page);
    DEBUG('f', "Physical page: %d\n", machine->tlb[page].physicalPage);
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

    if (page < 0)
        page = rand() % TLBSize; // Política de elección de página de TLB
        
    return page;           // Copiar la page a la pagetable actualizada
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
