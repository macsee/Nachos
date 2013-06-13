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
main(int argc, char** argv)
{	
	// char* algo = argv[0];
	//Write("Soy lasb\nEl argc es: ",22,1);
	//PrintNumber(argc);
	//Write("\n",1,1);
	int i;
	for (i = 0; i < argc; i++)
	{
		// Write("a",1,1);
		// PrintNumber(&argv[i]);
		// Write(argv[i],4,1);
    Write(argv[i],6,1);
    Write("\n",1,1);
	}
 //  Write("c\n",2,1);
 //  PrintNumber(&argv[1]);
 //  Write(argv[1],4,1);
	// // 3322
  
	Exit(66);
}
