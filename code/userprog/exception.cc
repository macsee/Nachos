// exception.cc 
//  Entry point into the Nachos kernel from user programs.
//  There are two kinds of things that can cause control to
//  transfer back to here from user code:
//
//  syscall -- The user code explicitly requests to call a procedure
//  in the Nachos kernel.  Right now, the only function we support is
//  "Halt".
//
//  exceptions -- The user code does something that the CPU can't handle.
//  For instance, accessing memory that doesn't exist, arithmetic errors,
//  etc.  
//
//  Interrupts (which can also cause control to transfer from user
//  code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "synchconsole.h"
#include "userfunctions.h"

#ifdef USE_TLB
#include "../vm/vm_utils.h"
#endif

class Thread;

/* Funciones definidas para el ejercicio 1 de la práctica 3
 */

#define INCREMENTAR_PC                      \
    int pc;                                 \
    pc = machine->ReadRegister(PCReg);      \
    machine->WriteRegister(PrevPCReg, pc);  \
    pc = machine->ReadRegister(NextPCReg);  \
    machine->WriteRegister(PCReg, pc);      \
    pc += 4;                                \
    machine->WriteRegister(NextPCReg, pc);  


void
RunProcess (void* arg) 
{        
    AddrSpace* space = (AddrSpace*) arg;   // Recuperando ejecutable guardado en space.
    OpenFile* exec = space->getExec();
    NoffHeader noffH = space->getNoffH();

    int codeSize = noffH.code.size;               // Recuperando noffH.code.size 
    int codeVirtAddr = noffH.code.virtualAddr;       // Recuperando noffH.code.virtualAddr
    int codeinFileAddr = noffH.code.inFileAddr;   // Recuperando noffH.code.inFileAddr
    // Escribiendo la parte de codigo del ejecutable en la memoria de usuario

    currentThread->space->InitRegisters(); 
    currentThread->space->RestoreState();
/*
    char* code = new char[codeSize];
    exec->ReadAt(code,codeSize, codeinFileAddr);
    writeBuffToUsr(code, codeVirtAddr, codeSize);
    delete code;

    // Terminada la copia de codigo a memoria de usuario

    int dataSize = noffH.initData.size;
    int dataVirtAddr = noffH.initData.virtualAddr;
    int datainFileAddr = noffH.initData.inFileAddr;


    char* data = new char[dataSize];
    exec->ReadAt(data,dataSize, datainFileAddr);
    writeBuffToUsr (data, dataVirtAddr, dataSize);
    delete data;
*/
    int sp = PageSize*currentThread->space->getNumPages();
    int argc = currentThread->getArgc();
    char** argv = currentThread->getArgv();
    int local_addr[argc];
    //int dir;

    machine->WriteRegister(4, argc);

    //printf("El stack en el comienzo esta en : %d\n", sp);
    
     for (int i = argc-1; i >= 0; i--)
    {
        //printf("El argumento nro %d, es %s\n", i, argv[i]);
        sp -= strlen(argv[i])+1; //hacemos lugar para copiar el arg i-esimo
        writeStrToUsr(argv[i], sp); //copiamos a mem el arg i-esimo
        local_addr[i] = sp; //direccion donde comienza el arg i-esimo
        //printf("El stack esta ahora en: %d\n", sp);
    }
    sp-=sp%4; // ESTO ES PARA QUE FUNCIONE TODO!!!! Necesito que el stack esté apuntando a una dirección que sea múltiplo de 4.

    for ( int i = argc-1; i >= 0; i-- ) 
    {
        sp -= 4; // hacemos lugar para escribir la direccion de memoria donde se encuentra el arg i-esimo
        machine->WriteMem(sp, 4, local_addr[i]); // escribimos la direccion de memoria del arg i-esimo
        //printf("Direccion de arg%d = %d\n", i, local_addr[i]);
    }

    machine->WriteRegister(5, sp);

    if (argc > 0) {

        for (int i = 0; i < NumTotalRegs; i++)
           DEBUG('j', "registers[%d] = %d\n", i, machine->ReadRegister(i) );

        for (int i = 0; i < MemorySize; i++)
           DEBUG('j',"MainMemory[%d] = %c - %d\n", i, machine->mainMemory[i], machine->mainMemory[i]);
    }  
    machine->WriteRegister(StackReg, sp-16);
    machine->Run();     // jump to the user progam
                        // machine->Run never returns;
          // the address space exits
          // by doing the syscall "exit"

}  

//----------------------------------------------------------------------
// ExceptionHandler
//  Entry point into the Nachos kernel.  Called when a user program
//  is executing, and either does a syscall, or generates an addressing
//  or arithmetic exception.
//
//  For system calls, the following is the calling convention:
//
//  system call code -- r2
//    arg1 -- r4
//    arg2 -- r5
//    arg3 -- r6
//    arg4 -- r7
//
//  The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//  "which" is the kind of exception.  The list of possible exceptions 
//  are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    char* buffer = NULL;
    int type = machine->ReadRegister(2);

    int arg1 = machine->ReadRegister(4);
    int arg2 = machine->ReadRegister(5);
    int arg3 = machine->ReadRegister(6);
    int arg4 = machine->ReadRegister(7);
  
    if (which == SyscallException) {
        switch ( type ) 
        {
            //////////////////////////// EXIT ///////////////////////////////////////////////////
            case SC_Exit : 
            {
                // Eliminar el PID
                //RemoveThreadFromTable(currentThread->getPid()); 
                // Vacias los espacios de memoria correspondientes
                // se puede poner el delete de space en el destructor de Thread y solo llamar a la 
                // funcion FInish()
                //delete currentThread->space;
                // Eliminar el Thread
                SetRetornoInTable (currentThread->getPid(), arg1);
                //TablaPid *tbPid = GetThreadFromTable(currentThread->getPid());
                DEBUG('f', "EXIT: thread con PID = %d hace EXIT. Retorno %d\n", currentThread->getPid(), arg1);
                currentThread->Finish();
                //Cleanup(); 
                break;
                // Que se hace con el estatus de exit
            }
            //////////////////////////// JOIN ///////////////////////////////////////////////////
            case SC_Join : 
            {
                
                TablaPid *tbPid = GetThreadFromTable(arg1);
                
                if (tbPid == NULL) {
                    machine->WriteRegister(2, -1);
                    DEBUG('f', "JOIN[Error]: Error en pid: %d", arg1);
                    break;
                }
                    
                DEBUG('f', "JOIN: Se aplica sobre thread con pid = %d\n", arg1);
                //currentThread->SaveUserState(); // guardo el status del currentThread antes de hacer el join y pasar al otro thread

                tbPid->thread->Join();
                int retorno = tbPid->retorno;

                //int retorno = PidTable[arg1].retorno; //retorno del Pid anterior 

                //currentThread->RestoreUserState(); //restauro lo que había guardado una vez que volvi
                DEBUG('f', "JOIN: Retorno: %d\n", retorno);
                machine->WriteRegister(2, retorno);
                //tbPid.thread->RestoreUserState();
                // Eliminar el PID
                //RemoveThreadFromTable(currentThread->getPid()); 
                // Vacias los espacios de memoria correspondientes
                // se puede poner el delete de space en el destructor de Thread y solo llamar a la 
                // funcion FInish()
                //delete currentThread->space;

                // Eliminar el Thread
                //currentThread->Finish();

                // Que se hace con el estatus de exit
                break;
            }
            case SC_Halt : 
            {        
                DEBUG('a', "Shutdown, initiated by user program.\n");
                interrupt->Halt();
                break;
            }

            case SC_Create : 
            {
                DEBUG('a', "Create file, initiated by user program.\n");

                buffer = new char[128];
                readStrFromUsr(arg1, buffer);

                if (buffer == NULL) {
                    DEBUG('f', "CREATE[Error]: Error al crear archivo: %s\n", buffer);
                    break;   
                }    
                //ASSERT (fileSystem->Create(buffer, 128));

                if (!fileSystem->Create(buffer, 128)) {
                    DEBUG('f', "CREATE[Error]: No se puede crear el archivo\n");
                    break;
                }

                DEBUG('f', "CREATE: Se creó el archivo: %s\n", buffer);

                delete buffer;
                    
                break;
                //    writeStrToUsr("Hola", addrStr);
                //    readBuffFromUsr(addrStr, buffer, 10);
            }
            //////////////////////////// OPEN ///////////////////////////////////////////////////
            case SC_Open : 
            {
                DEBUG('a', "Open file, initiated by user program.\n");

                buffer = new char[128];
                readStrFromUsr(arg1, buffer);
                int fd;

                if (buffer == NULL)
                    break;

                OpenFile* of = fileSystem->Open(buffer);

                if (of == NULL) {
                    fd = -1;
                    DEBUG('f', "OPEN[Error]: No pudo abrirse el archivo %s\n", buffer);
                }  
                else {
                    fd = currentThread->AddFileToTable(of);
                    DEBUG('f', "OPEN: Abriendo archivo %s con file id = %d\n", buffer, fd);
                }
                  
                machine->WriteRegister(2, fd);

                delete buffer;
                
                break;
            }
            ///////////////////////// CLOSE ///////////////////////////////////////////////////
            case SC_Close : 
            {
                DEBUG('a', "Close file, initiated by user program.\n");

                //ASSERT (arg3 >= 0);

                if (arg1 < 0) { //por si se produjo algún otro error
                    DEBUG('f', "CLOSE[Error]: Error desconocido\n");
                    break;
                }

                //ASSERT(currentThread->RemoveFileFromTable(arg1));

                if (!currentThread->RemoveFileFromTable(arg1))
                    DEBUG('f', "CLOSE[Error]: Error en file id: %d\n", arg1);
                else
                    DEBUG('f', "CLOSE: Cerrando el archivo con file id = %d\n", arg1);

                break;
            }
            /////////////////////////// WRITE ///////////////////////////////////////////////////
            case SC_Write : 
            {
                DEBUG('a', "Write, initiated by user program.\n");

                //ASSERT (arg3 >= 0);

                if (arg3 < 0) { //por si se produjo algún otro error
                    DEBUG('f', "WRITE: Error desconocido\n");
                    break;
                }

                FileDescriptor* fd = currentThread->GetFileIDFromTable(arg3);

                if (fd == NULL) {
                    DEBUG('f', "WRITE[Error]: Error en file id\n");
                    
                    break;
                }
                 
                if (fd->modo == FD_R) {
                    DEBUG('f', "WRITE[Error]: No se puede escribir en un dispositivo de lectura\n");
                    
                    break;
                }
/*
                if (!fd->valido) {
                    DEBUG('f', "WRITE[Error]: El file id: %d no existe\n", arg3);
                    
                    break;
                }
*/
                buffer = new char[arg2];
                readBuffFromUsr(arg1, buffer, arg2);

                if (buffer == NULL)
                    break;

                if (fd->consola) {
                    for (int i = 0; i < arg2; i++) {
                      synchconsole->WriteToConsole(buffer[i]);
                    }
                }
                else {  ///////////////////// LEYENDO DESDE ARCHIVO
                    DEBUG('f', "WRITE: Escribiendo en archivo con file id = %d\n", arg3);
                    fd->openfile->Write(buffer, arg2);
                }

                delete buffer;
                
                break;
            }
            //////////////////////////////////////////// READ /////////////////////////////////////////////////       
            case SC_Read : 
            {
                DEBUG('a', "Read file, initiated by user program.\n");

                if (arg3 < 0) { //por si se produjo algún otro error
                    machine->WriteRegister(2, -1);
                    break;
                }

                FileDescriptor* fd = currentThread->GetFileIDFromTable(arg3);

                if (fd == NULL) {
                    DEBUG('f', "READ[Error]: Error en file id\n");
                    machine->WriteRegister(2, -1);
                    break;
                }

                int bytes_leidos;

                if (fd->modo == FD_W) {
                    DEBUG('f', "READ[Error]: No se puede leer de un dispositivo de escritura\n");
                    machine->WriteRegister(2, -1);
                    
                    break;
                }
/*
                if (!fd->valido) {
                    DEBUG('f', "READ[Error]: El file id: %d no es valido\n", arg3);
                    machine->WriteRegister(2, -1);
                    
                    break;
                }
*/
                buffer = new char[arg2];

                if (fd->consola) {
                    char ch;
                    int i = 0;
                    while (i < arg2) {
                        ch = synchconsole->ReadFromConsole();
                        buffer[i] = ch;
                        i++;
                    }
                    writeBuffToUsr(buffer, arg1, arg2);
                    bytes_leidos = strlen(buffer);
                }
                else {  ///////////////////// LEYENDO DESDE ARCHIVO
                    DEBUG('f', "READ: Leyendo del archivo con file id = %d\n", arg3);
                    bytes_leidos = fd->openfile->Read(buffer, arg2);
                    writeBuffToUsr(buffer, arg1, arg2);
                    //printf("%d bytes leídos\n", bytes_leidos);
                }

                delete buffer;
                machine->WriteRegister(2, bytes_leidos);
                
                break;
            }
            /////////////////////////////// EXEC ////////////////////////////////////////////////      
            case SC_Exec : 
            {
                DEBUG('a', "Exec, initiated by user program.\n");
            
                buffer = new char[128];
                readStrFromUsr(arg1, buffer);

                if (buffer == NULL)
                    break;

                OpenFile *executable = fileSystem->Open(buffer);
                AddrSpace *space;

                if (executable == NULL) {
                    DEBUG('f', "EXEC[Error]: No es posible ejecutar el programa %s\n", buffer);
                    machine->WriteRegister(2, -1);
                    break;
                }

                DEBUG('f', "EXEC: Ejecutando el prgrama %s\n", buffer);
            
                Thread* thread;
                
                if (arg4 == 0) 
                    thread = new Thread(buffer, false);
                else
                    thread = new Thread(buffer, true);        

                space = new AddrSpace(executable);    
                thread->space = space;

                if (arg2 > 0) {
                    thread->SetArgs(arg2, arg3);
                }

                //space->InitRegisters(); 
                //space->RestoreState();

                thread->Fork(RunProcess, (void*) space);

                //delete executable;      // close file
                //delete buffer;
                int pid = AddThreadToTable(thread);
                machine->WriteRegister(2, pid);
                break;
            }
            /////////////////////// DEFAULT ///////////////////////////////////////////////////
            default : 
            {
                printf("Unexpected user mode exception %d %d\n", which, type);
                ASSERT(false);
                break;                
            }
        }
        INCREMENTAR_PC;
    }
    else if (which == PageFaultException) {
        int virAddrReq = machine->ReadRegister(BadVAddrReg);
        DEBUG('f', "Page Fault Exception, virtual address %d not found.\n", virAddrReq);

        #ifdef USE_TLB
            pageFaultHandler(virAddrReq);
        #endif
    }
    else if (which == ReadOnlyException) {
        printf("Se produjo una excepción de tipo ReadOnlyException\n");
    }    
}