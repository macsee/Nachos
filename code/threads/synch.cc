// synch.cc
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"
#include "scheduler.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(const char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List<Thread*>;
}

//----------------------------------------------------------------------
// Semaphore::~Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    DEBUG('s', "%s Haciendo P\n",currentThread->getName()); // Línea de debug

    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts

    while (value == 0)   			// semaphore not available
    {
        queue->Append(currentThread);		// so go to sleep
        currentThread->Sleep();
    }
    value--; 					// semaphore available,
    // consume its value

    interrupt->SetLevel(oldLevel);		// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    DEBUG('s', "%s Haciendo V\n",currentThread->getName()); // Línea de debug

    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = queue->Remove();
    if (thread != NULL)    // make thread ready, consuming the V immediately
        scheduler->ReadyToRun(thread);
    value++;
    interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments
// Note -- without a correct implementation of Condition::Wait(),
// the test case in the network assignment won't work!
Lock::Lock(const char* debugName)
{
    name = debugName;
    sem = new Semaphore(debugName, 1);
    thread = NULL;
}

Lock::~Lock()
{
    delete sem;
}

void Lock::Acquire()
{
    ASSERT(!isHeldByCurrentThread());
    if(thread != NULL)
        if (thread->GetPriority() < currentThread->GetPriority()) {
			DEBUG('s',"%s intenta tomar el Lock %s pero es poseído por %s\n", currentThread->getName(), name, thread->getName());
            DEBUG('s',"+++El thread %s cambia prioridad del thread %s de %d a %d +++\n", currentThread->getName(), thread->getName(), thread->GetPriority(), currentThread->GetPriority());
            
            scheduler->UpdateScheduler(thread,currentThread->GetPriority());
            //scheduler->Print(); 
        }
    
    sem->P();
    DEBUG('s',"%s toma el Lock %s\n", currentThread->getName(), name);
    thread = currentThread;
}
void Lock::Release()
{
    ASSERT(isHeldByCurrentThread());
    DEBUG('s',"%s deja el Lock %s\n", thread->getName(), name);
	if (thread->GetOldPriority() != thread->GetPriority()) {
		DEBUG('s',"Restaurando prioridad del thread %s de %d a %d\n", thread->getName(), thread->GetPriority(), thread->GetOldPriority());
		//scheduler->UpdateScheduler(thread,thread->GetOldPriority());
        thread->SetPriority(thread->GetOldPriority());		
	}
	thread = NULL;	
    sem->V();
}

bool Lock::isHeldByCurrentThread()
{
    return (thread == currentThread);
}

Condition::Condition(const char* debugName, Lock* conditionLock)
{
    name = debugName;
    miLock = conditionLock; // conditionLock lock asosciado a la región crítica, por ejemplo Buffer
    listaSem = new List<Semaphore*>;
}

Condition::~Condition()
{
    delete listaSem;
}

/*
    Atómicamente libera el Lock asociado  y bloquea (incondicionalmente) el hilo llamante.
    Este hilo, al ser despertado, debe pedir el lock nuevamente.
*/
void Condition::Wait()
{
    ASSERT(miLock->isHeldByCurrentThread());
    Semaphore* sem = new Semaphore("semCond",0);
    listaSem->Append(sem);
    miLock->Release();
    sem->P();
    miLock->Acquire();
    delete sem;
}

/*
    USAR LISTA DE SEMAFOROS EN VEZ DE LISTA DE THREADS. USAR UN p() CON SEMAFORO 0 PARA MANDAR A DORMIR Y USAR UN v()
 PARA DESPERTAR AL THREAD
 vER EJEMPLO DE CONSUMIDOR Y PRODUCTOR PARA PROBAR

    Sólo despierta UN hilo (si lo hubiera). NO libera el Lock asosciado.
    El llamante del metodo debe ser el lock asociado, para esto utilizo un assert.
*/
void Condition::Signal()
{
    if (!listaSem->IsEmpty())
    {
        ASSERT(miLock->isHeldByCurrentThread());
        listaSem->Remove()->V();
    }
}
/*
    Despierta TODOS hilos bloqueados.
    Tampoco libera el lock asociadod.
*/
void Condition::Broadcast()
{
    ASSERT (miLock->isHeldByCurrentThread());
    while (!listaSem->IsEmpty())
        Signal();
}

