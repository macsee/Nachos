#include "syscall.h"

int
main(int argc, char** argv)
{	
	char* algo = argv[0];
	int i = 0;
	Write("Soy lasb\n",10,1);
	Write(algo,4,1);
	Exit(66);
}
