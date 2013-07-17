#ifndef COREMAP_H
#define COREMAP_H

#include "system.h"
#include "machine.h"
#include "addrspace.h"
class Thread;

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
		void GetPage(int vpage);
		int GetVPage(int ppage); //Devuelve la DV asociada
		// Addrspace* GetOwner(int process);
		AddrSpace* GetSpace(int process);
		AddrSpace* GetOwner(int ppage);
		int GetVpage (int ppage);
		bool IsFree(int ppage);
		void Update(int ppage, int vpage);
		void UpdateArgs(int ppage, int vpage, Thread* thread);
		void PrintCoremap();
		Core* GetCoremapEntry(int ppage);
		void IncCount(int ppage) { 
			mapaDeNucleo[ppage].count ++; 
			// printf("+++ Encontre la pag %d - Pos = %d Count = %d\n", mapaDeNucleo[ppage].virtualPage, ppage, mapaDeNucleo[ppage].count);
		}
		int GetPageFifo() { 
			// printf("Politica Fifo devuelve nextPage=%d\n", nextPage);
			return nextPage; 
		}
	private:
		//List<*Thread>owners; //NO ESTA USADO
		Core* mapaDeNucleo;	//arreglo de estructuras Core
		int nextPage;
};


#endif // COREMAP_H

