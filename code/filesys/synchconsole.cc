#include "synchconsole.h"


static void
WriteRequestDone (void* arg)
{
    SynchConsole* noconsole = (SynchConsole *)arg;
    noconsole->WriteDone();
}

static void
ReadRequestDone (void* arg)
{
    SynchConsole* noconsole = (SynchConsole *)arg;
    noconsole->ReadAvail();
}

//===============================================================================================================//
//===============================================================================================================//

SynchConsole::SynchConsole(char* in, char* out)
{
    console = new Console(in, out, ReadRequestDone, WriteRequestDone, (void*) this);
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);
    lock = new Lock("synch console lock");
}

SynchConsole::~SynchConsole()
{
    delete console;
    delete readAvail;
    delete writeDone;
    delete lock;
}

char
SynchConsole::ReadFromConsole()
{   
    char ch;
    lock->Acquire();
        readAvail->P();
        ch = console->GetChar();        // Queda esperando hasta que llegue algun caracter para leer.
    lock->Release();                  // Cuando llega un caracter, el objeto consola ejecuta automaticamente el callback a ReadRequestDone
    return ch;                          // entonces ReadRequestDone despierta al (los) thread(s) que esta(n) esperando para leer. (?)
}

void
SynchConsole::WriteToConsole(const char ch)
{
    lock->Acquire();
        console->PutChar(ch);
        writeDone->P();
    lock->Release();
}

