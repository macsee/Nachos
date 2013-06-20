#include "syscall.h"

int
main(int argc, char** argv)
{	
	char buffer;

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
