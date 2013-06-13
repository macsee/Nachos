#include "syscall.h"

void PrintNumber (int x)
{
  const int MAX = 30;
  char string[MAX+1];
  int current = MAX; 

  if ( x == 0 )
  {
    Write("0",1,1);
    return;
  }
 
  string[current] = 0; 
  while ( x != 0 )
  {
    string[--current] = (x%10) + '0';
    x /= 10;
    if (current==0) return;
  }  

  Write(string+current,MAX-current,1);
}

int
main()
{
    SpaceId newProc;
    OpenFileId input = ConsoleInput;
    OpenFileId output = ConsoleOutput;
    char prompt[2], ch, buffer[60];
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
	// Inicio cambios en el intérprete
			int argc = 0;
			char** argv[60];
			char* proc = &buffer[0];
			int j;

			for (j = 0; j < 60; j++) {
				// Write("Hola", 4, output);
				if ( buffer[j] == ' ' ) {
					buffer[j] = '\0';
					argv[argc] = &buffer[j+1];
					argc++;
				}
			}

			if (argc >= 1) {
				newProc = Exec(proc,argc,argv);
			}
			else {
	// Fin cambios en el intérprete
				newProc = Exec(proc, 0, (char**)'\0');
			}
			Join(newProc);
	    }
	}
}