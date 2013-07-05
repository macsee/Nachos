#include "system.h"
#include "coremap.h"
#include "machine.h"


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
}

int
Coremap::GetPPage(int vpage){
	int minim = mapaDeNucleo[0].count;
	int k = 0;

	for (int i = 1; i < NumPhysPages; ++i)
	{
		if (minim >= mapaDeNucleo[i].count){
			minim = mapaDeNucleo[i].count;
			k = i;
			if (minim == 0)
				break;
		}
	}
	
	if (mapaDeNucleo[k].count > 0){ 
		//Verifico si el lugar estaba ocupado por otra pag
		DEBUG('k', "Iniciando Swapping\n");

		//Verifico que la pag a sacar sea del Thread que esta intentando hacer el swap y la busco en la TLB para quitarla
		if (mapaDeNucleo[k].thread->getPid() == currentThread->getPid())
		{
			for (int i = 0; i < TLBSize; i++)
			{
				if (machine->tlb[i].virtualPage == vpage)
					machine->tlb[i].valid = false;
			}
			
		}
		

		mapaDeNucleo[k].thread->space->SaveToSwap(mapaDeNucleo[k].virtualPage);	

	}
	//Si Valid = True => Buscar si 
	//							PhysPage > 0 => en MainMemory
	//							PhysPage = -1 => DemandLoading 
	//Si Valid = False (Phys no deberia ser < 0)=> La PhysPage esta en swap

	//Asigno los valores la entrada de mapaDeNucleo elegida
	mapaDeNucleo[k].thread = currentThread;
	mapaDeNucleo[k].virtualPage = vpage;
	mapaDeNucleo[k].count ++;
	
	return k;
}


Coremap::~Coremap(){
	delete mapaDeNucleo;
}

void
Coremap::Clear(int ppage){
	mapaDeNucleo[ppage].virtualPage = -1;
	mapaDeNucleo[ppage].thread = NULL;
}

#endif
