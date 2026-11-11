#ifndef INDICE_OPERADOR_H_INCLUDED
#define INDICE_OPERADOR_H_INCLUDED

#include "arbol.h"

#define TAM_NOMBRE 100

#define CLAVE_NO_ENCONTRADA 0
#define CLAVE_ENCONTRADA 1
#define ARCHIVO_NO_INDEXADO 0
#define ARCHIVO_INDEXADO 1

typedef struct
{
    char nombreOperador[TAM_NOMBRE+1];
    unsigned indiceRegistro;
} tIndice;

typedef struct
{
    char nombre[TAM_NOMBRE + 1];
    int puntajeTotal;
    int jornadasTotal;
} tOperador;

int compararClaveIndice(const void *a, const void *b);
int buscarEnArchivoConIndice(FILE *fp, const tArbol *p, tOperador *o);
int guardarEnArchivoConIndice(FILE *fp, const tArbol *pa, tOperador *o);

int indexarArchivoOperador(FILE *fp, tArbol *pa);

int guardarIndiceEnArchivoPreOrden(const tArbol *p, FILE *fp);

int cargarIndiceDesdeArchivo(tArbol *pa, FILE *fp);
void guardarNodoEnArchivo(void *info, unsigned tam, unsigned n, void *params);

#endif // INDICE_OPERADOR_H_INCLUDED
