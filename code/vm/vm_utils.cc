#ifdef USE_TLB
#include "vm_utils.h"

void pageFaultHandler(int virAddrReq) {

	stats->numTLBMiss++;
	stats->numMemAccess--; // Se debe restar el accesso a memoria ya que en un pagefault se re intenta leer esa dirccion de memoria hasta que se tiene éxito.
	int virPageReq = virAddrReq/PageSize;

	int page = getTLBentry();

    machine->tlb[page] = currentThread->space->getPage(virPageReq);     
   
    //DEBUG('f', "Virtual Page: %d\n", virPageReq);           
    DEBUG('f', "Direccion virtual: %d\n", virAddrReq);
    DEBUG('f', "Pagina de la TLB: %d\n", page);
    DEBUG('f', "Physical page: %d\n\n", machine->tlb[page].physicalPage);            

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
#endif
