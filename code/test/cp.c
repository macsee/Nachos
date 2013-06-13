#include "syscall.h"

int
main(int argc, char** argv)
{	
	char buffer[128];
		Create(argv[1]);
		int origen = Open(argv[0]);
		int destino = Open(argv[1]);

		while ( Read(buffer, 1, origen ) != 0 ) {
			Write(buffer,1,destino);
		}
		Write("Se copio el archivo con exito\n",30,ConsoleOutput);
		Close(origen);
		Close(destino);
	
	Exit(70);

}
