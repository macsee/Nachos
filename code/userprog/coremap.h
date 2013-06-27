#ifndef COREMAP_H
#define COREMAP_H

#include "system.h"

class Coremap {
	public:

		Coremap();
		~Coremap();
		int GetPPage();
		int GetVPage(int ppage);
		Addrspace* GetOwner(int process);
	private:

};


#endif // COREMAP_H

