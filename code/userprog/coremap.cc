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
	numpage = 0;
}

int
Coremap::GetPageFIFO() {
	int pagina;

	if (numpage < NumPhysPages) {
		pagina = numpage;
	}
	else {
		numpage = 0;
		pagina = numpage;
	}
	numpage++;
	return pagina;
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

int
Coremap::GetPageLRUPerfecto(){
	int maximo = 0;
	int k = 0;


	for (int i = 0; i < NumPhysPages; i++)
	{
		if (maximo < mapaDeNucleo[i].count) {
			maximo = mapaDeNucleo[i].count;
			k = i;
		} else {
			mapaDeNucleo[i].count ++;
		}
	}

	PrintCoremap();
	printf("Uso la pagina %d\n", k);
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
	//mapaDeNucleo[ppage].count ++;
	 mapaDeNucleo[ppage].count = 0;
}

void Coremap::PrintCoremap() {

	for (int i = 0; i < NumPhysPages; ++i) {

		if (mapaDeNucleo[i].thread == NULL)
			DEBUG('h',"Coremap[%d] = {owner : NULL | count : %d}\n", i, mapaDeNucleo[i].count);
		else
			DEBUG('h',"Coremap[%d] = {owner : %d | count : %d}\n", i, mapaDeNucleo[i].thread->getPid(), mapaDeNucleo[i].count);
		// mapaDeNucleo[i]=Core(NULL,0,-1);
	}

}

Core* Coremap::GetCoremapEntry(int ppage) {

	return &mapaDeNucleo[ppage];
}

void Coremap::CleanEntries(Thread* thread) {

	for (int i = 0; i < NumPhysPages; ++i) {

		if (mapaDeNucleo[i].thread == thread){
			mapaDeNucleo[i].virtualPage = -1;
			mapaDeNucleo[i].thread = NULL;
			mapaDeNucleo[i].count = 0;
		}
	}	
}

#endif
