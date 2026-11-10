#ifndef OPERADOR_H_INCLUDED
#define OPERADOR_H_INCLUDED

#include "stdio.h"

#define TAM_NOMBRE 100

typedef struct
{
    char nombre[TAM_NOMBRE + 1];
    int puntajeTotal;
    int jornadasTotal;
} tOperador;

void mostrarOperador(const tOperador *p);

#endif // OPERADOR_H_INCLUDED
