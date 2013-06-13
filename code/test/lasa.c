#include "syscall.h"

int
main()
{	
	char *argv[] = {"../userprog/file1\0","../userprog/file2\0"};
	//char** i = &h;
	
	int pid = Exec("../test/cat", 2, argv);

	// while(1) {
	// 	Write("a\n", 1, ConsoleOutput);
	// }

	Exit(65);
}
