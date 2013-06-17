#include "syscall.h"

int
main()
{
    SpaceId newProc;
    OpenFileId input = ConsoleInput;
    OpenFileId output = ConsoleOutput;
    char prompt[2], ch, buffer[128];
    int argc = 0;
	char* argv[60];
	char* proc;
    int i;

    prompt[0] = '-';
    prompt[1] = '-';

    while( 1 )
    {
		Write(prompt, 2, output);
		i = 0;
		
		do {
		    Read(&buffer[i], 1, input); 
		} while( buffer[i++] != '\n' );

		buffer[--i] = '\0';

		if( i > 0 ) {

			if (buffer[0] == '&') {
				proc = &buffer[1];
			} else {
				proc = &buffer[0];
			}
			
			int j;
			for (j = 0; j < 128; j++) {
				if ( buffer[j] == ' ' ) {
					buffer[j] = '\0';
					argv[argc] = &buffer[j+1];
					argc++;
				}
			}

			if (proc[0] == 'q') {
				Halt();
			}
			
			if (argc >= 1) {
				if (buffer[0] != '&') {
					//Write("Llamando Exec!\n",15,ConsoleOutput);
					newProc = Exec(proc, argc, argv,1);
					Join(newProc);					
				}
				else
					newProc = Exec(proc, argc, argv,0);	

			}
			else {
				if (buffer[0] != '&') {
					//Write("Llamando Exec!\n",15,ConsoleOutput);
					newProc = Exec(proc, 0, (char**)'\0', 1);
					Join(newProc);
				}	
				else
					newProc = Exec(proc, 0, (char**)'\0', 0);		
			}
			argc = 0;
			
	    }
	}
}
