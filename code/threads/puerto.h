#ifndef PUERTO_H
#define PUERTO_H

#include "synch.h"

class Puerto
{
public:
    Puerto(const char* debugName);
    ~Puerto();
    void Send(int mensaje);
    void Receive(int* mensaje);
    const char* getName()
    {
        return name;
    }

private:
    const char* name;
    Lock* lockPuerto;
    Condition* hay_mensaje;
    Condition* esperan_mensaje;
    int msj;
    bool mensajes_listos;
    int receptores_esperando;
    int emisores_esperando;
};

#endif // PUERTO_H
