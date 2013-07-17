#include "system.h"
#include "coremap.h"
#include "machine.h"
#include "vm_utils.h"


#ifdef USE_TLB

Coremap::Coremap(){
	mapaDeNucleo = new Core[NumPhysPages];
	for (int i = 0; i < NumPhysPages; ++i)
	{
		mapaDeNucleo[i].virtualPage = -1;
		mapaDeNucleo[i].count = 0;
		mapaDeNucleo[i].thread = NULL;
		// mapaDeNucleo[i]=Core(NULL,0,-1);
	}
	nextPage = 0;
}

int
Coremap::GetPageLRU(){
	int minim = mapaDeNucleo[0].count;
	int k = 0;

	for (int i = 0; i < NumPhysPages; i++)
	{
		if (minim > mapaDeNucleo[i].count) {

			minim = mapaDeNucleo[i].count;
			k = i;
			if (minim == 0)
				break;

		}
	}
	// printf("Politica LRU devuelve min count=%d de k=%d\n", mapaDeNucleo[k].count, k);
	return k;
	//Si Valid = True => Buscar si 
	//							PhysPage > 0 => en MainMemory
	//							PhysPage = -1 => DemandLoading 
	//Si Valid = False (Phys no deberia ser < 0)=> La PhysPage esta en swap

	//Asigno los valores la entrada de mapaDeNucleo elegida
}


Coremap::~Coremap(){
	delete mapaDeNucleo;
}

void
Coremap::Clear(int ppage){
	mapaDeNucleo[ppage].virtualPage = -1;
	mapaDeNucleo[ppage].thread = NULL;
}


AddrSpace* Coremap::GetOwner(int ppage) {

	return mapaDeNucleo[ppage].thread->space;
}

int Coremap::GetVpage(int ppage) {

	return mapaDeNucleo[ppage].virtualPage;
}

bool Coremap::IsFree(int ppage) {
	return (mapaDeNucleo[ppage].thread == NULL);
}

void Coremap::Update(int ppage, int vpage) {
	mapaDeNucleo[ppage].virtualPage = vpage;
	mapaDeNucleo[ppage].thread = currentThread;
	mapaDeNucleo[ppage].count ++;
	// mapaDeNucleo[ppage].count = 1;
	 // printf(">>> Meto la pagina %d - Pos = %d Count = %d\n", vpage, ppage, mapaDeNucleo[ppage].count);
	
	nextPage = (nextPage+1)%NumPhysPages;
}

void Coremap::UpdateArgs(int ppage, int vpage, Thread* thread) {
	mapaDeNucleo[ppage].virtualPage = vpage;
	mapaDeNucleo[ppage].thread = thread;
	mapaDeNucleo[ppage].count ++;	
}

void Coremap::PrintCoremap() {

	for (int i = 0; i < NumPhysPages; ++i) {

		if (mapaDeNucleo[i].thread == NULL)
			DEBUG('k',"Coremap[%d] = {owner : NULL | count : %d}\n", i, mapaDeNucleo[i].count);
		else
			DEBUG('k',"Coremap[%d] = {owner : %d | count : %d}\n", i, mapaDeNucleo[i].thread->getPid(), mapaDeNucleo[i].count);
		// mapaDeNucleo[i]=Core(NULL,0,-1);
	}

}

Core* Coremap::GetCoremapEntry(int ppage) {

	return &mapaDeNucleo[ppage];
}

// void Coremap::GetPage(int vpage) {
    
//     AddrSpace* owner_space;
//     int phys_page = -1;
//     int owner_vpage = -1;
//     int page = -1;

//     TranslationEntry* page_entry = currentThread->space->getPage(vpage);
   
//     phys_page = coreMap->GetPageLRU();
//     DEBUG('k', "Physical page %d selected by algorithm!\n",phys_page);
//     // Si la phys_page esta libre no hacemos nada.
//     // De lo contrario tenemos que swapear.
//     if (!coreMap->IsFree(phys_page)) {
//         owner_space = coreMap->GetOwner(phys_page);
//         owner_vpage = coreMap->GetVpage(phys_page);

//         // chequeamos si la phys_page estaba en la TLB
//         // y si es así la marcamos para quitar, pero antes la copiamos.
//         page = getTLBindex(phys_page);
//         if (page >= 0)
//             owner_space->CopyTLBtoPageTable(page);
//             // No seria conveniente chequear el campo dirty para ver si conviene guardar la pagina??
            

//         owner_space->SaveToSwap(owner_vpage); // Guardamos en SWAP
    
//     }
//     else
//         DEBUG('k', "No swapping needed!\n");

//     page_entry->physicalPage = phys_page;

//     // Actualizamos el Coremap con los datos nuevos
//     coreMap->Update(phys_page, vpage);
    
// }

#endif
