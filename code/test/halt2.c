/* halt.c
 *	Simple program to test whether running a user program works.
 *	
 *	Just do a "syscall" that shuts down the OS.
 *
 * 	NOTE: for some reason, user programs with global data structures 
 *	sometimes haven't worked in the Nachos environment.  So be careful
 *	out there!  One option is to allocate data structures as 
 * 	automatics within a procedure, but if you do this, you have to
 *	be careful to allocate a big enough stack to hold the automatics!
 */

#include "syscall.h"

int
mainF()
{	
	char buffer[128];
	int size;
	Create("file1");
	int file_id = Open("file1");
	Write("Hello World\n",12,file_id);
	Close(file_id);
	Write("Ingrese nombre de archivo: ",27,ConsoleOutput);
	Read(buffer, 5, ConsoleInput );
	file_id = Open(buffer);
	size = Read(buffer, 15, file_id );
	//file_id = Open(buffer);
	//Read(buffer, 15, file_id );
	//Close(file_id);
	Write(buffer,size,ConsoleOutput);
	//Exec("../test/file_test");
    Halt();
    /* not reached */
}
