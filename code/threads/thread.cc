// thread.cc
//	Routines to manage threads.  There are four main operations:
//
//	Fork -- create a thread to run a procedure concurrently
//		with the caller (this is done in two steps -- first
//		allocate the Thread object, then call Fork on it)
//	Finish -- called when the forked procedure finishes, to clean up
//	Yield -- relinquish control over the CPU to another ready thread
//	Sleep -- relinquish control over the CPU, but thread is now blocked.
//		In other words, it will not run again, until explicitly
//		put back on the ready queue.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "thread.h"
#include "switch.h"
#include "synch.h"
#include "puerto.h"
#include "system.h"
#include "list.h"
#include "userfunctions.h"

// this is put at the top of the execution stack,
// for detecting stack overflows
const unsigned STACK_FENCEPOST = 0xdeadbeef;

//----------------------------------------------------------------------
// Thread::Thread
// 	Initialize a thread control block, so that we can then call
//	Thread::Fork.
//
//	"threadName" is an arbitrary string, useful for debugging.
//----------------------------------------------------------------------

Thread::Thread(const char* threadName, bool isJoinable, int prioridad)
{
    name = threadName;
    stackTop = NULL;
    stack = NULL;
    status = JUST_CREATED;
    
    joinable = isJoinable;
	pri = prioridad;
	old_pri = pri;
    if (joinable)
        joinPuerto = new Puerto("Puerto Apache");
	
#ifdef USER_PROGRAM
	space = NULL;

    FileDescriptor console_in;
    console_in.openfile = NULL;
    console_in.consola = true;
    console_in.valido = true;
    console_in.modo = FD_R;
    tablaDesc.push_back(console_in);
    // *FileDescriptor(NULL, true, true, FD_R)

    FileDescriptor console_out;
    console_out.openfile = NULL;
    console_out.consola = true;
    console_out.valido = true;
    console_out.modo = FD_W;
    tablaDesc.push_back(console_out);

#endif
}

//----------------------------------------------------------------------
// Thread::~Thread
// 	De-allocate a thread.
//
// 	NOTE: the current thread *cannot* delete itself directly,
//	since it is still running on the stack that we need to delete.
//
//      NOTE: if this is the main thread, we can't delete the stack
//      because we didn't allocate it -- we got it automatically
//      as part of starting up Nachos.
//----------------------------------------------------------------------

Thread::~Thread()
{
    DEBUG('t', "Deleting thread \"%s\"\n", name);

    ASSERT(this != currentThread);

    if (stack != NULL)
        DeallocBoundedArray((char *) stack, StackSize * sizeof(HostMemoryAddress));
    #ifdef USER_PROGRAM
        delete space;
    #endif


}

//----------------------------------------------------------------------
// Thread::Fork
// 	Invoke (*func)(arg), allowing caller and callee to execute
//	concurrently.
//
//	NOTE: although our definition allows only a single integer argument
//	to be passed to the procedure, it is possible to pass multiple
//	arguments by making them fields of a structure, and passing a pointer
//	to the structure as "arg".
//
// 	Implemented as the following steps:
//		1. Allocate a stack
//		2. Initialize the stack so that a call to SWITCH will
//		cause it to run the procedure
//		3. Put the thread on the ready queue
//
//	"func" is the procedure to run concurrently.
//	"arg" is a single argument to be passed to the procedure.
//----------------------------------------------------------------------

void
Thread::Fork(VoidFunctionPtr func, void* arg)
{
#ifdef HOST_x86_64
    DEBUG('t', "Forking thread \"%s\" with func = 0x%lx, arg = %ld\n",
          name, (HostMemoryAddress) func, arg);
#else
    // code for 32-bit architectures
    DEBUG('t', "Forking thread \"%s\" with func = 0x%x, arg = %d\n",
          name, (HostMemoryAddress) func, arg);
#endif

    StackAllocate(func, arg);
    printf("Stack en: %d\n", &stack );

    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    scheduler->ReadyToRun(this);	// ReadyToRun assumes that interrupts
    // are disabled!
    interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------
// Thread::CheckOverflow
// 	Check a thread's stack to see if it has overrun the space
//	that has been allocated for it.  If we had a smarter compiler,
//	we wouldn't need to worry about this, but we don't.
//
// 	NOTE: Nachos will not catch all stack overflow conditions.
//	In other words, your program may still crash because of an overflow.
//
// 	If you get bizarre results (such as seg faults where there is no code)
// 	then you *may* need to increase the stack size.  You can avoid stack
// 	overflows by not putting large data structures on the stack.
// 	Don't do this: void foo() { int bigArray[10000]; ... }
//----------------------------------------------------------------------

void
Thread::CheckOverflow()
{
    if (stack != NULL)
    {
        ASSERT(*stack == STACK_FENCEPOST);
    }
}

//----------------------------------------------------------------------
// Thread::Finish
// 	Called by ThreadRoot when a thread is done executing the
//	forked procedure.
//
// 	NOTE: we don't immediately de-allocate the thread data structure
//	or the execution stack, because we're still running in the thread
//	and we're still on the stack!  Instead, we set "threadToBeDestroyed",
//	so that Scheduler::Run() will call the destructor, once we're
//	running in the context of a different thread.
//
// 	NOTE: we disable interrupts, so that we don't get a time slice
//	between setting threadToBeDestroyed, and going to sleep.
//----------------------------------------------------------------------

//
void
Thread::Finish ()
{
    interrupt->SetLevel(IntOff);
    ASSERT(this == currentThread);
    
    DEBUG('t', "Finishing thread \"%s\"\n", getName());

    if (joinable) {
        joinPuerto->Send(1); // Necesitamos enviar un numero distinto de cero
        DEBUG('t', "Finishing thread Joinable  \"%s\"\n", getName());       
    }

    #ifdef USER_PROGRAM
        RemoveThreadFromTable(pid);
        if (!MoreThreadsToRun ()) {
            stats->Print();
            Cleanup();
        }
        // Chequear si no quedan más threads en la lista de threads antes de hacer Cleanup
    #endif

    threadToBeDestroyed = currentThread;
    Sleep(); // invokes SWITCH
    // not reached
}


void Thread::Join ()
{
	ASSERT(joinable);
	int estado;
    joinPuerto->Receive(&estado);
	delete joinPuerto;
}

//----------------------------------------------------------------------
// Thread::Yield
// 	Relinquish the CPU if any other thread is ready to run.
//	If so, put the thread on the end of the ready list, so that
//	it will eventually be re-scheduled.
//
//	NOTE: returns immediately if no other thread on the ready queue.
//	Otherwise returns when the thread eventually works its way
//	to the front of the ready list and gets re-scheduled.
//
//	NOTE: we disable interrupts, so that looking at the thread
//	on the front of the ready list, and switching to it, can be done
//	atomically.  On return, we re-set the interrupt level to its
//	original state, in case we are called with interrupts disabled.
//
// 	Similar to Thread::Sleep(), but a little different.
//----------------------------------------------------------------------

void
Thread::Yield ()
{
    Thread *nextThread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    ASSERT(this == currentThread);

    DEBUG('t', "Yielding thread \"%s\"\n", getName());

    nextThread = scheduler->FindNextToRun();
    if (nextThread != NULL)
    {
        scheduler->ReadyToRun(this);
        scheduler->Run(nextThread);
    }
    interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------
// Thread::Sleep
// 	Relinquish the CPU, because the current thread is blocked
//	waiting on a synchronization variable (Semaphore, Lock, or Condition).
//	Eventually, some thread will wake this thread up, and put it
//	back on the ready queue, so that it can be re-scheduled.
//
//	NOTE: if there are no threads on the ready queue, that means
//	we have no thread to run.  "Interrupt::Idle" is called
//	to signify that we should idle the CPU until the next I/O interrupt
//	occurs (the only thing that could cause a thread to become
//	ready to run).
//
//	NOTE: we assume interrupts are already disabled, because it
//	is called from the synchronization routines which must
//	disable interrupts for atomicity.   We need interrupts off
//	so that there can't be a time slice between pulling the first thread
//	off the ready list, and switching to it.
//----------------------------------------------------------------------
void
Thread::Sleep ()
{
    Thread *nextThread;

    ASSERT(this == currentThread);
    ASSERT(interrupt->getLevel() == IntOff);

    DEBUG('t', "Sleeping thread \"%s\"\n", getName());

    status = BLOCKED;
    while ((nextThread = scheduler->FindNextToRun()) == NULL)
    {
        interrupt->Idle();	// no one to run, wait for an interrupt
    }

    scheduler->Run(nextThread); // returns when we've been signalled
}

//----------------------------------------------------------------------
// ThreadFinish, InterruptEnable
//	Dummy functions because C++ does not allow a pointer to a member
//	function.  So in order to do this, we create a dummy C function
//	(which we can pass a pointer to), that then simply calls the
//	member function.
//----------------------------------------------------------------------

static void ThreadFinish()
{
    currentThread->Finish();
}
static void InterruptEnable()
{
    interrupt->Enable();
}

//----------------------------------------------------------------------
// Thread::StackAllocate
//	Allocate and initialize an execution stack.  The stack is
//	initialized with an initial stack frame for ThreadRoot, which:
//		enables interrupts
//		calls (*func)(arg)
//		calls Thread::Finish
//
//	"func" is the procedure to be forked
//	"arg" is the parameter to be passed to the procedure
//----------------------------------------------------------------------

void
Thread::StackAllocate (VoidFunctionPtr func, void* arg)
{
    stack = (HostMemoryAddress *) AllocBoundedArray(StackSize * sizeof(HostMemoryAddress));

    // i386 & MIPS & SPARC stack works from high addresses to low addresses
    stackTop = stack + StackSize - 4;	// -4 to be on the safe side!

    // the 80386 passes the return address on the stack.  In order for
    // SWITCH() to go to ThreadRoot when we switch to this thread, the
    // return addres used in SWITCH() must be the starting address of
    // ThreadRoot.
    *(--stackTop) = (HostMemoryAddress)ThreadRoot;

    *stack = STACK_FENCEPOST;

    machineState[PCState] = (HostMemoryAddress) ThreadRoot;
    machineState[StartupPCState] = (HostMemoryAddress) InterruptEnable;
    machineState[InitialPCState] = (HostMemoryAddress) func;
    machineState[InitialArgState] = (HostMemoryAddress) arg;
    machineState[WhenDonePCState] = (HostMemoryAddress) ThreadFinish;
}

#ifdef USER_PROGRAM
#include "machine.h"

//----------------------------------------------------------------------
// Thread::SaveUserState
//	Save the CPU state of a user program on a context switch.
//
//	Note that a user program thread has *two* sets of CPU registers --
//	one for its state while executing user code, one for its state
//	while executing kernel code.  This routine saves the former.
//----------------------------------------------------------------------

void
Thread::SaveUserState()
{
    for (int i = 0; i < NumTotalRegs; i++)
        userRegisters[i] = machine->ReadRegister(i);
}

//----------------------------------------------------------------------
// Thread::RestoreUserState
//	Restore the CPU state of a user program on a context switch.
//
//	Note that a user program thread has *two* sets of CPU registers --
//	one for its state while executing user code, one for its state
//	while executing kernel code.  This routine restores the former.
//----------------------------------------------------------------------

void
Thread::RestoreUserState()
{
    for (int i = 0; i < NumTotalRegs; i++)
        machine->WriteRegister(i, userRegisters[i]);
}

// Definimos las 3 funciones de manejo del Descriptor

// AddFileToTable retorna la posición del file descriptor agregado; 


int
Thread::AddFileToTable (OpenFile* op)
{
	FileDescriptor fd;
    int i = 0;

    std::vector<FileDescriptor>:: iterator it;

    for( it = tablaDesc.begin(); it != tablaDesc.end() ; it++ ) {
        if (!it->valido && !it->consola) { 
            // it es un puntero a elementos de un vector, en este caso, a FileDescriptors (fd). Para Mauro
            it->openfile = op;
            it->consola = false;
            it->valido = true;
            it->modo = FD_RW;
            return i;
        }
        i++;
    }    

    fd.openfile = op;
    fd.consola = false;
    fd.valido = true;
    fd.modo = FD_RW;

	tablaDesc.push_back(fd);
    return i;
}

// Cfdamfdbiofd de tipo de retorno int por void

bool
Thread::RemoveFileFromTable (OpenFileId of)
{
    if (  of < 0 || of >= tablaDesc.size() ) {
        DEBUG('f', "Error al intentar remover archivo de la tabla de descriptores. File id %d inexistente\n", of);
        return false;
    }

    if ( !tablaDesc[of].valido ) {
        DEBUG('f', "Error al intentar remover archivo de la tabla de descriptores. File id %d no valido\n", of);
        return false;
    }

    if ( tablaDesc[of].consola ) {
        DEBUG('f', "Error al intentar remover archivo de la tabla de descriptores. File id prohibido\n");
        return false;
    }

    tablaDesc[of].valido = false;
    tablaDesc[of].openfile = NULL;
    return true;
}

FileDescriptor 
Thread::GetFileIDFromTable (OpenFileId of)
{
    if (  of < 0 || of >= tablaDesc.size() ) {
        DEBUG('f', "Error al intentar obtener archivo de la tabla de descriptores. File id %d inexistente\n", of);
    }

    else if ( !tablaDesc[of].valido ) {
        DEBUG('f', "Error al intentar obtener archivo de la tabla de descriptores. File id %d no valido\n", of);
    }
    else {
        return tablaDesc[of];
    }
}

void 
Thread::SetArgs(int argc, int argv)
{
    args = new char*[argc];
    char* ptr = new char[128];

    printf("El stack pointer esta %d\n", stack);

    readStrFromUsr(argv, ptr);
    int i = 0;
    ptr = strtok(ptr," ");
    while(ptr != NULL)
    {
        args[i] = ptr;
        ptr = strtok(NULL, " ");
        printf("EL ARGUMENTO %d, es %s\n", i, args[i]);
        i++; 
    }

}

#endif
