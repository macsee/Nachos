#include "syscall.h"

int
main(int argc, char** argv)
{	
	char buffer[1024];
	int i;
	for (i = 0; i < argc; i++) {
		int file_id = Open(argv[i]);
		while ( Read(buffer, 1, file_id ) != 0 ) {
			Write(buffer,1,ConsoleOutput);
		}
		Close(file_id);
	}
	Exit(65);

}
