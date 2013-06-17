#include "syscall.h"

int
main(int argc, char** argv)
{	
	char buffer;
	int i;
	//char num = '0'+argc;
	//Write(&num,1,ConsoleOutput);
	//Write("\n",1,ConsoleOutput);

	if (argc < 1) {
		Write("Modo de uso: cat [archivo1] [archivo2] .. [archivoN]\n", 53, ConsoleOutput);
		Exit(-1);
	}	

	for (i = 0; i < argc; i++) {
		int file_id = Open(argv[i]);
		if (file_id < 0) {
			Write("Alguno de los archivos proporcionados no existe\n", 48, ConsoleOutput);
			Exit(-1);
		}	
		//Write(argv[i],12,ConsoleOutput);
		//Write("\n",1,ConsoleOutput);
		while ( Read(&buffer, 1, file_id ) != 0 ) {
			Write(&buffer,1,ConsoleOutput);
		}
		Close(file_id);
	}
	Exit(65);

}
