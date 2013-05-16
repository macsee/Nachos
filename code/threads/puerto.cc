#include "synch.h"
#include "system.h"

Puerto::Puerto(const char* debugName)
{
    name = debugName;
    lockPuerto = new Lock ("lock_puerto");
    hay_mensaje = new Condition("hay_mensaje", lockPuerto);
    esperan_mensaje = new Condition("esperan mensaje", lockPuerto);
    mensajes_listos = false;
    receptores_esperando = 0;
    emisores_esperando = 0;
}

Puerto::~Puerto()
{
    delete hay_mensaje;
    delete esperan_mensaje;
	delete lockPuerto;
}

void Puerto::Send(int mensaje)
{
    lockPuerto->Acquire();
    emisores_esperando++;
    while (receptores_esperando == 0 || mensajes_listos)
    {
        if (receptores_esperando == 0)    
            DEBUG('q',"*** El thread %s está esperando un receptor y se duerme... \n", currentThread->getName());
        if (mensajes_listos)
            DEBUG('q',"*** El thread %s está esperando que se desocupe el puerto y se duerme... \n", currentThread->getName());
        esperan_mensaje->Wait();
        DEBUG('q',"*** El thread %s se despierta... \n", currentThread->getName());
    }
    msj = mensaje;
    DEBUG('q',"*** El thread %s envia el mensaje: %d... \n", currentThread->getName(),msj);
    mensajes_listos = true;
    emisores_esperando--;
    hay_mensaje->Signal();
    //DEBUG('q',"*** El thread %s hizo signal... \n", currentThread->getName());
    lockPuerto->Release();
}

void Puerto::Receive(int* mensaje)
{
    lockPuerto->Acquire();
    receptores_esperando++;
    DEBUG('q',"*** El thread %s responde como receptor... \n", currentThread->getName());
    while (!mensajes_listos)
    {
        if (emisores_esperando > 0){
            esperan_mensaje->Signal();
        }
        DEBUG('q',"*** El thread %s se duerme... \n", currentThread->getName());
        hay_mensaje->Wait();
        DEBUG('q',"*** El thread %s se despierta... \n", currentThread->getName());
    }

    *mensaje = msj;
    DEBUG('q',">>> El thread %s recibe mensaje: %d... \n", currentThread->getName(),msj);
    mensajes_listos = false;
    receptores_esperando--;
    if (emisores_esperando > 0){
        esperan_mensaje->Signal();
    }
    lockPuerto->Release();
}

