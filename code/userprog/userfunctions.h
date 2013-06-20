#ifndef USERFUNCTIONS_H
#define USERFUNCTIONS_H

void readStrFromUsr(int usrAddr, char* outStr);
void readBuffFromUsr(int usrAddr, char *outBuff, int byteCount);
void writeStrToUsr(char *str, int usrAddr);
void writeBuffToUsr(char *str, int usrAddr, int byteCount);
void printMainMemory();

#endif // USERFUNCTIONS_H