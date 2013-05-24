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


class Thread;

/* Funciones definidas para el ejercicio 1 de la práctica 3
 */

#define INCREMENTAR_PC						\
	int pc;									\
	pc = machine->ReadRegister(PCReg);		\
	machine->WriteRegister(PrevPCReg, pc);	\
	pc = machine->ReadRegister(NextPCReg);	\
	machine->WriteRegister(PCReg, pc);		\
	pc += 4;								\
	machine->WriteRegister(NextPCReg, pc);	


void 
readStrFromUsr(int usrAddr, char* outStr)
{
  int i = 0;
  int value = 1;

  while (value != '\0')
  {
    ASSERT(machine->ReadMem(usrAddr + i, 1, &value));
    outStr[i] = (char)value;
    i++;
  }
}

void 
readBuffFromUsr(int usrAddr, char *outBuff, int byteCount)
{
  int i = 0;
  int value = 1;

  while (i < byteCount)
  {
    ASSERT(machine->ReadMem(usrAddr + i, 1, &value));
    outBuff[i] = (char)value;
    i++;  
  }
}

void 
writeStrToUsr(char *str, int usrAddr)
{
  int i = 0;

  while (str[i] != '\0')
  {
    ASSERT(machine->WriteMem(usrAddr + i, 1, (int) str[i]));
    i++;
  }
  ASSERT(machine->WriteMem(usrAddr + i, 1, '\0'));
}

void 
writeBuffToUsr(char *str, int usrAddr, int byteCount)
{
  int i = 0;

  while (i < byteCount)
  {
    ASSERT(machine->WriteMem(usrAddr + i, 1, (int) str[i]));
    i++;
  }
}

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

    currentThread->space->InitRegisters(); 
    currentThread->space->RestoreState();

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
  
  if (which == SyscallException) {
    switch ( type ) {
      case SC_Halt :        
        DEBUG('a', "Shutdown, initiated by user program.\n");
        interrupt->Halt();
        break;

      case SC_Create : {
        DEBUG('a', "Create file, initiated by user program.\n");

        buffer = new char[128];
        readStrFromUsr(arg1, buffer);

		    ASSERT (fileSystem->Create(buffer, 128));

        DEBUG('f', "Se creó el archivo %s\n", buffer);

        delete buffer;
		    INCREMENTAR_PC;
        break;
        //    writeStrToUsr("Hola", addrStr);
        //    readBuffFromUsr(addrStr, buffer, 10);
      }
////////////////////////////////////////////////////////// OPEN ///////////////////////////////////////////////////
      case SC_Open : {
        DEBUG('a', "Open file, initiated by user program.\n");

        buffer = new char[128];
        readStrFromUsr(arg1, buffer);
        int fd;

        OpenFile* of = fileSystem->Open(buffer);

        if (of == NULL) {
          fd = -1;
          DEBUG('f', "No pudo abrirse el archivo %s\n", buffer);
        }  
        else {
          fd = currentThread->AddFileToTable(of);
          DEBUG('f', "Abriendo archivo %s con file id = %d\n", buffer, fd);
        }
          
        machine->WriteRegister(2, fd);

        delete buffer;
        INCREMENTAR_PC;
        break;
      }
////////////////////////////////////////////////////////// CLOSE ///////////////////////////////////////////////////
      case SC_Close : {
        DEBUG('a', "Close file, initiated by user program.\n");

        ASSERT (arg3 >= 0);

        ASSERT(currentThread->RemoveFileFromTable(arg1));

        //if (!currentThread->RemoveFileFromTable(arg1))
        //  DEBUG('f', "El file id: %d no existe\n", arg1);
        //else {
        DEBUG('f', "Cerrando el archivo con file id = %d\n", arg1);
        //}
      
        INCREMENTAR_PC;
        break;
      }
////////////////////////////////////////////////////////// WRITE ///////////////////////////////////////////////////
      case SC_Write : {
        DEBUG('a', "Write, initiated by user program.\n");

        ASSERT (arg3 >= 0);

        FileDescriptor fd = currentThread->GetFileIDFromTable(arg3);

        if (fd.modo == FD_R) {
            DEBUG('f', "No se puede escribir en un dispositivo de lectura\n");
            INCREMENTAR_PC;
            break;
        }

        if (!fd.valido) {
            DEBUG('f', "El file id: %d no existe\n", arg3);
            INCREMENTAR_PC;
            break;
        }

        buffer = new char[arg2];
        readBuffFromUsr(arg1, buffer, arg2);

        if (fd.consola) {
            for (int i = 0; i < arg2; i++) {
              synchconsole->WriteToConsole(buffer[i]);
            }
        }
        else {  ///////////////////// LEYENDO DESDE ARCHIVO
            DEBUG('f', "Escribiendo en archivo con file id = %d\n", arg3);
            fd.openfile->Write(buffer, arg2);
        }

        delete buffer;
        INCREMENTAR_PC;
        break;
      }
////////////////////////////////////////////////////////// READ /////////////////////////////////////////////////       
      case SC_Read : {
        DEBUG('a', "Read file, initiated by user program.\n");

        if (arg3 < 0) { //por si se produjo algún otro error
          machine->WriteRegister(2, -1);
          INCREMENTAR_PC;
          break;
        }

        FileDescriptor fd = currentThread->GetFileIDFromTable(arg3);
        int bytes_leidos;

        if (fd.modo == FD_W) {
            DEBUG('f', "No se puede leer de un dispositivo de escritura\n");
            machine->WriteRegister(2, -1);
            INCREMENTAR_PC;
            break;
        }

        if (!fd.valido) {
            DEBUG('f', "El file id: %d no es valido\n", arg3);
            machine->WriteRegister(2, -1);
            INCREMENTAR_PC;
            break;
        }

        buffer = new char[arg2];

        if (fd.consola) {
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
            
            DEBUG('f', "Leyendo del archivo con file id = %d\n", arg3);
            bytes_leidos = fd.openfile->Read(buffer, arg2);
            writeBuffToUsr(buffer, arg1, arg2);
            printf("%d bytes leídos\n", bytes_leidos);
            
        }

        delete buffer;
        machine->WriteRegister(2, bytes_leidos);
        INCREMENTAR_PC;
        break;
      }
//////////////////////////////////////////////////////////// EXEC ////////////////////////////////////////////////      
      case SC_Exec : {
        DEBUG('a', "Exec, initiated by user program.\n");
    
        buffer = new char[128];
        readStrFromUsr(arg1, buffer);

        OpenFile *executable = fileSystem->Open(buffer);
        AddrSpace *space;

        if (executable == NULL) {
          DEBUG('f', "No es posible ejecutar el archivo %s\n", buffer);
          machine->WriteRegister(2, -1);
          INCREMENTAR_PC;
          break;
        }

        DEBUG('f', "Ejecutando el archivo %s\n", buffer);
    
        Thread* thread = new Thread(buffer);    

        space = new AddrSpace(executable);    
        thread->space = space;

        //space->InitRegisters(); 
        //space->RestoreState();

        thread->Fork(RunProcess, (void*) space);

        //delete executable;      // close file
        delete buffer;
        
        //machine->WriteRegister(2, (int)space);
        INCREMENTAR_PC;
        break;
      }
////////////////////////////////////////////////////////// DEFAULT ///////////////////////////////////////////////////
      default :
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(false);
        break;                
    }
  }
}
