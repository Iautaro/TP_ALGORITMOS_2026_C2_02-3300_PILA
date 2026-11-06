#ifndef ARBOL_H_INCLUDED
#define ARBOL_H_INCLUDED

#include<string.h>
#include<stdlib.h>
#include<stdio.h>

/** etiquetas típicas de TDA (salvo para primitivas booleanas) **/
#define CLA_DUP 1
#define SIN_MEM 2
#define TODO_OK 0
#define HALLADO 1
#define NO_HALLADO 0

#define MINIMO(X,Y) ((X)<(Y)?(X):(Y))

#define reservarMemoriaNodo( X , Y , Z , W ) ( \
 ( ( X ) = (typeof( X ))malloc( Y ) ) == NULL || \
 ( ( Z ) = malloc( W ) ) == NULL ? \
 free( X ), 0 : 1 )

typedef struct sNodoArbol
{
    void *info;
    unsigned tamInfo;
    struct sNodoArbol *izq,
               *der;
} tNodoArbol;

typedef tNodoArbol *tArbol;

void crearArbol(tArbol *pa);
void eliminarArbol(tArbol *pa);
int insertarArbolBinBusq(tArbol *pa,
                         const void *d,
                         unsigned tam,
                         int (*cmp)(const void *, const void *));
int buscarEnArbol(const tArbol* pa,
                  void *d,
                  unsigned tam,
                  int (*cmp)(const void *, const void *));
void recorrerArbolPreOrden(const tArbol *pa,
                           unsigned n,
                           void *params,
                           void (*accion)(void *, unsigned, unsigned, void *));

void recorrerArbolInOrden(const tArbol * p,
                          unsigned n,
                          void *params,
                          void (*accion)(void *, unsigned, unsigned, void *));


#endif // ARBOL_H_INCLUDED
