#include "syscall.h"

int
main()
{	
	int pid = Exec("../test/lasb", 2, "hola so");

	// while(1) {
	// 	Write("a\n", 1, ConsoleOutput);
	// }

	Exit(65);
}
