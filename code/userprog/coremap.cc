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
Coremap::GetPageLRU(){
	int minim = mapaDeNucleo[0].count;
	int k = 0;

	for (int i = 0; i < NumPhysPages; i++)
	{
		if (minim >= mapaDeNucleo[i].count) {

			minim = mapaDeNucleo[i].count;
			k = i;
			if (minim == 0)
				break;

		}
	}
	
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
}

void Coremap::PrintCoremap() {

	for (int i = 0; i < NumPhysPages; ++i) {

		if (mapaDeNucleo[i].thread == NULL)
			printf("Coremap[%d] = {owner : NULL | count : %d}\n", i, mapaDeNucleo[i].count);
		else
			printf("Coremap[%d] = {owner : %d | count : %d}\n", i, mapaDeNucleo[i].thread->getPid(), mapaDeNucleo[i].count);
		// mapaDeNucleo[i]=Core(NULL,0,-1);
	}

}

Core* Coremap::GetCoremapEntry(int ppage) {

	return &mapaDeNucleo[ppage];
}

#endif
