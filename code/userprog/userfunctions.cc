#include "userfunctions.h"
#include "system.h"


void 
readStrFromUsr(int usrAddr, char* outStr)
{
  int i = 0;
  int value = 1;

  while (value != '\0')
  {
    if (machine->ReadMem(usrAddr + i, 1, &value)) {
      outStr[i] = (char)value;
      i++;  
    }
    else {
      DEBUG('f', "readStrFromUsr: Error al leer de memoria: %d\n", usrAddr);
      outStr = NULL;
      break;
    }  
  }
}

void 
readBuffFromUsr(int usrAddr, char *outBuff, int byteCount)
{
  int i = 0;
  int value = 1;

  while (i < byteCount)
  {
    if (machine->ReadMem(usrAddr + i, 1, &value)) {
      outBuff[i] = (char)value;
      i++;
    }
    else {
      DEBUG('f', "readBuffFromUsr: Error al leer de memoria: %d\n", usrAddr);
      outBuff = NULL;
      break;
    }  
  }
}

void 
writeStrToUsr(char *str, int usrAddr)
{
  int i = 0;

  while (str[i] != '\0')
  {
    if (machine->WriteMem(usrAddr + i, 1, (int) str[i]))
      i++;
    else {
      DEBUG('f', "writeStrToUsr: Error al escribir en memoria: %d\n", usrAddr);
      break;
    }
  }

  machine->WriteMem(usrAddr + i, 1, '\0');
}

void 
writeBuffToUsr(char *str, int usrAddr, int byteCount)
{
  int i = 0;

  while (i < byteCount)
  {
    if (machine->WriteMem(usrAddr + i, 1, (int) str[i]))
      i++;
    else {
      DEBUG('f', "writeBuffToUsr: Error al escribir en memoria: %d\n", usrAddr);
      break;
    }
  }
}

void 
printMainMemory()
{
  for (int i = 0; i < NumTotalRegs; i++)
    DEBUG('j', "registers[%d] = %d\n", i, machine->ReadRegister(i));

  for (int i = 0; i < MemorySize; i++)
    DEBUG('j',"MainMemory[%d] = %c - %d\n", i, machine->mainMemory[i], machine->mainMemory[i]);
}