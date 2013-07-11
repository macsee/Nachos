#ifndef COREMAP_H
#define COREMAP_H

#include "system.h"
#include "machine.h"
#include "addrspace.h"

struct Core
{
	Thread* thread;
	int count;			//veces usado
	int virtualPage;
};


class Coremap {
	public:
		Coremap();
		~Coremap();
		void Clear(int ppage);
		int GetPageLRU();  //Devuelve la DF que libera de la MM 
		int GetVPage(int ppage); //Devuelve la DV asociada
		// Addrspace* GetOwner(int process);
		AddrSpace* GetSpace(int process);
		AddrSpace* GetOwner(int ppage);
		int GetVpage (int ppage);
		bool IsFree(int ppage);
		void Update(int ppage, int vpage);
		void PrintCoremap();
		Core* GetCoremapEntry(int ppage);   
	private:
		//List<*Thread>owners; //NO ESTA USADO
		Core* mapaDeNucleo;	//arreglo de estructuras Core
};


#endif // COREMAP_H

