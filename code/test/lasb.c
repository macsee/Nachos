#include "syscall.h"

int
main()
{	
	int i = 0;
	while(i < 10) {
		Write("b\n", 1, ConsoleOutput);
		i++;
	}
	Exit(66);
}
