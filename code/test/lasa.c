#include "syscall.h"

int
main()
{	
	char *argv[] = {"hola\0","mundo\0"};
	//char** i = &h;
	
	int pid = Exec("../test/lasb", 2, argv);


	// while(1) {
	// 	Write("a\n", 1, ConsoleOutput);
	// }

	Exit(65);
}
