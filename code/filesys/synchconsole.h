#ifndef SYNCHCONSOLE_H
#define SYNCHCONSOLE_H

#include "system.h"
#include "console.h"
#include "addrspace.h"
#include "synch.h"

class SynchConsole {
  public:
    SynchConsole(char* in, char* out);    	
					
    ~SynchConsole();
    
    char ReadFromConsole ();
    				
    void WriteToConsole(const char ch);
    
    void RequestDone();

    void WriteDone() { writeDone->V(); };

    void ReadAvail() { readAvail->V(); };

  private:
    Console *console;	
    Semaphore *readAvail;
    Semaphore *writeDone;					
    Lock *lock_read;
	Lock *lock_write;							
};

#endif
