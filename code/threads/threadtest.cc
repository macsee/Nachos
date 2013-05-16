// threadtest.cc
//	Simple test case for the threads assignment.
//
//	Create several threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield,
//	to illustrate the inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.
//
// Parts from Copyright (c) 2007-2009 Universidad de Las Palmas de Gran Canaria
//

#include "copyright.h"
#include "system.h"
#include "synch.h" // Librería para utilizar semáforos, lock y variables de condicion
#include "puerto.h"

#ifdef SEMAPHORE_TEST
    Semaphore* s = new Semaphore("MySemaphore", 3); // Creamos el semáforo con valor inicial 3
#else
    Lock* miLock = new Lock("MyLock");
#endif

Puerto* pto = new Puerto ("Puerto");

//----------------------------------------------------------------------
// SimpleThread
//  Loop 10 times, yielding the CPU to another ready thread
//  each iteration.
//
//  "name" points to a string with a thread name, just for
//      debugging purposes.
//----------------------------------------------------------------------

void
SimpleThread(void* name)
{
    // Reinterpret arg "name" as a string
    char* threadName = (char*)name;

    // If the lines dealing with interrupts are commented,
    // the code will behave incorrectly, because
    // printf execution may cause race conditions.
#ifdef SEMAPHORE_TEST
    s->P();
#else
    miLock->Acquire();
#endif

    for (int num = 0; num < 10; num++)
    {
        //IntStatus oldLevel = interrupt->SetLevel(IntOff);
        printf("*** thread %s looped %d times\n", threadName, num);
        //interrupt->SetLevel(oldLevel);
        //scheduler->Print();
        //currentThread->Yield();
    }

#ifdef SEMAPHORE_TEST
    s->V();
#else
    miLock->Release();
#endif

    //IntStatus oldLevel = interrupt->SetLevel(IntOff);
    printf(">>> Thread %s has finished\n", threadName);
    //interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------
// MediumThread
// 
//----------------------------------------------------------------------

void
MediumThread(void* name)
{
    // Reinterpret arg "name" as a string
    char* threadName = (char*)name;
    printf("Thread %s comenzando....\n", threadName);
    while(1)
    {
        //i++;
        //printf("Soy %s !!!\n", threadName);
    }
	printf(">>> Thread %s has finished\n", threadName);
}

//----------------------------------------------------------------------
// LowThread
// 
//----------------------------------------------------------------------

void
LowThread(void* name)
{
	printf("Soy el thread %s y adquirí el lock %s\n", currentThread->getName(), miLock->getName());

	miLock->Acquire();

    // Reinterpret arg "name" as a string
    char* threadName = (char*)name;
    
    char* threadname1 = new char[100];
    sprintf(threadname1, "Hilo High Priority");

    Thread* newThread1 = new Thread (threadname1,false, 4);
    newThread1->Fork (SimpleThread, (void*)threadname1);

    char* threadname3 = new char[100];
    sprintf(threadname3, "Hilo Medium Priority");

    Thread* newThread3 = new Thread (threadname3,false, 2);
    newThread3->Fork (MediumThread, (void*)threadname3);
    
    currentThread->Yield();

    for (int num = 0; num < 10; num++)
    {
        printf("*** thread %s trabajando.....\n", threadName);
        
    }
	
    miLock->Release();

	printf(">>> Thread %s has finished\n", threadName);

}

//----------------------------------------------------------------------
// Sender
// 
//----------------------------------------------------------------------

void Sender(void* valor)
{
    pto->Send(*(int*)valor);
}

//----------------------------------------------------------------------
// Receiver
// 
//----------------------------------------------------------------------

void Receiver(void* nombre_puerto)
{
    int msj;
    pto->Receive(&msj);
}


//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between several threads, by launching
//	ten threads which call SimpleThread, and finally calling
//	SimpleThread ourselves.
//----------------------------------------------------------------------
/*
void
ThreadTest()
{
    DEBUG('t', "Entering SimpleTest");

    for ( int k=1; k<=4; k++) {
      char* threadname = new char[100];
      sprintf(threadname, "Hilo %d", k);

      Thread* newThread = new Thread (threadname,false, k);
      newThread->Fork (SimpleThread, (void*)threadname);
    }
    // system.cc:    currentThread = new Thread("main"); modificar la creacion de este
    // thread para darle una prioridad distinta a 0
    SimpleThread( (void*)"Hilo 0");
}
*/
/*
// ThreadTest para la prueba de puertos utilizando funciones Sender y Receiver definidas arriba
// Probarlo con la bandera -q de DEBUG
void
ThreadTest()
{
    int *kk = new int[5];
    
    for ( int k=1; k<=4; k++)
    {
        kk[k]=k;

        char* emisor = new char[100];
        sprintf(emisor, "Emisor %d", k);
        Thread* t1 = new Thread (emisor);
        t1->Fork (Sender, (void*)&kk[k]);
    }
    
    for ( int k=1; k<=3; k++)
    {
        char* receptor = new char[100];
        sprintf(receptor, "Receptor %d", k);
        Thread* t2 = new Thread (receptor);
        t2->Fork (Receiver, NULL);

    }
}
*/

// Prueba del Join()
/*
void
ThreadTest()
{
        Thread* t1 = new Thread ("Hijo",false,2);
        t1->Fork (SimpleThread, (void*)"Hijo");
        //t1->Join();
        printf("****** Soy el Padre... %s\n *******", currentThread->getName());
}
*/

void
ThreadTest()
{
    DEBUG('t', "Entering SimpleTest");

    char* threadname2 = new char[100];
    sprintf(threadname2, "Hilo Low Priority");

    Thread* newThread2 = new Thread (threadname2,false, 1);
    newThread2->Fork (LowThread, (void*)threadname2);

    
    DEBUG('t', "Exiting SimpleTest");
}

