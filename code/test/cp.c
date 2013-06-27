#include "syscall.h"

int
main(int argc, char** argv)
{	
	char buffer;
	// char num = '0'+argc;
	// char num2 = '0';
	// if (&argv < 1000) {
	// 	num2 = num2 + 1;
	// }
	// Write(&num,1,ConsoleOutput);
	// Write("\n",1,ConsoleOutput);
	// Write(&num2,1,ConsoleOutput);
	// Write("\n",1,ConsoleOutput);

	if (argc < 2) {
		Write("Modo de uso: cp [origen] [destino]\n", 35, ConsoleOutput);
		Exit(-1);
	}	

	Create(argv[1]);
	int origen = Open(argv[0]);
	int destino = Open(argv[1]);

	if (origen < 0) {
		Write("El archivo de origen no existe\n", 31, ConsoleOutput);
		Exit(-1);
	}	
	else {
		while ( Read(buffer, 1, origen ) != 0 ) {
			Write(buffer,1,destino);
		}
		Write("Se copio el archivo con exito\n",30,ConsoleOutput);
	}
	Close(origen);
	Close(destino);
	
	Exit(70);
}
