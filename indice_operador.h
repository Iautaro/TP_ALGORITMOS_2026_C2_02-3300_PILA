#ifndef INDICE_OPERADOR_H_INCLUDED
#define INDICE_OPERADOR_H_INCLUDED

#include "arbol.h"
#include "operador.h"

#define CLAVE_NO_ENCONTRADA 0
#define CLAVE_ENCONTRADA 1
#define ARCHIVO_NO_INDEXADO 0
#define ARCHIVO_INDEXADO 1

typedef struct
{
    char nombreOperador[TAM_NOMBRE+1];
    unsigned indiceRegistro;
} tIndice;

int compararClaveIndice(const void *a, const void *b);
void mostrarIndice(const tIndice *pi, unsigned nivel);
int buscarEnArchivoConIndice(FILE *fp, const tArbol *p, tOperador *o);
int guardarEnArchivoConIndice(FILE *fp, const tArbol *pa, tOperador *o);

int indexarArchivoOperador(FILE *fp, tArbol *pa);

void mostrarArbolIndice(const tArbol *p);
void mostrarNodoIndice(const tIndice *p);

int guardarIndiceEnArchivoPreOrden(const tArbol *p, FILE *fp);
int guardarIndiceEnArchivoInOrden(const tArbol *p, FILE *fp);

int cargarIndiceDesdeArchivo(tArbol *pa, FILE *fp);
void guardarNodoEnArchivo(void *info, unsigned tam, unsigned n, void *params);

#endif // INDICE_OPERADOR_H_INCLUDED
