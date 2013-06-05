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
main()
{	
	int pid = 0, ret;
	/*char buffer[128];
	Create("file1");
	int file_id = Open("file1");*/
	Write("Soy Halt\n",12,ConsoleOutput);
	/*Close(file_id);
	file_id = Open("file1");
	Read(buffer, 15, file_id );
	//file_id = Open(buffer);
	//Read(buffer, 15, file_id );
	//Close(file_id);
	Write(buffer,14,ConsoleOutput);*/
	// pid = Exec("../test/test");
	// ret ='a'+Join(pid);
	Write("Finalizando Halt\n",18,ConsoleOutput);
	Write(&ret,1,ConsoleOutput);
    //Halt();
    /* not reached */
	Exit(1);
}
