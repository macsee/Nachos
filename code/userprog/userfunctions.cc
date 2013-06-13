#include "userfunctions.h"
#include "system.h"


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
printMainMemory()
{
  for (int i = 0; i < NumTotalRegs; i++)
    DEBUG('j', "registers[%d] = %d\n", i, machine->ReadRegister(i));

  for (int i = 0; i < MemorySize; i++)
    DEBUG('j',"MainMemory[%d] = %c - %d\n", i, machine->mainMemory[i], machine->mainMemory[i]);
}