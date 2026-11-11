#include "arbol.h"


void crearArbol(tArbol *pa)
{
    *pa = NULL;
}

void eliminarArbol(tArbol *pa)
{
    if(*pa)
    {
        eliminarArbol(&(*pa)->izq);
        eliminarArbol(&(*pa)->der);

        free((*pa)->info);
        free(*pa);
        *pa = NULL;
    }
}

int insertarArbolBinBusq(tArbol *pa,
                         const void *d,
                         unsigned tam,
                         int (*cmp)(const void *, const void *))
{
    tNodoArbol *nue;
    int rc;

    if(*pa)
    {
        if((rc = cmp(d, (*pa)->info)) < 0)
            return insertarArbolBinBusq(&(*pa)->izq, d, tam, cmp);
        if (rc > 0)
            return insertarArbolBinBusq(&(*pa)->der, d, tam, cmp);

        memcpy((*pa)->info, d, tam);
        return CLA_DUP;
    }
    if(!reservarMemoriaNodo(nue, sizeof(tNodoArbol), nue->info, tam))
        return SIN_MEM;
    nue->tamInfo = tam;
    memcpy(nue->info, d, tam);
    nue->der = NULL;
    nue->izq = NULL;
    *pa = nue;
    return TODO_OK;
}


int buscarEnArbol(const tArbol* pa,
                  void *d,
                  unsigned tam,
                  int (*cmp)(const void *, const void *))
{

    if(!*pa)
        return NO_HALLADO;


    int comp = cmp(d, (*pa)->info);


    if(comp == 0)

    {

        memcpy(d, (*pa)->info, tam);

        return HALLADO;

    }


    if(comp < 0)

        return buscarEnArbol(&(*pa)->izq, d, tam, cmp);

    else

        return buscarEnArbol(&(*pa)->der, d, tam, cmp);
}

void recorrerArbolPreOrden(const tArbol *pa,
                           unsigned n,
                           void *params,
                           void (*accion)(void *, unsigned, unsigned, void *))
{
    if(!*pa)
        return;

    accion((*pa)->info, (*pa)->tamInfo, n, params);
    recorrerArbolPreOrden(&(*pa)->izq, n+1, params, accion);
    recorrerArbolPreOrden(&(*pa)->der, n+1, params, accion);
}

void recorrerArbolInOrden(const tArbol * p,
                          unsigned n,
                          void *params,
                          void (*accion)(void *, unsigned, unsigned, void *))
{
    if(!*p)
        return;

    recorrerArbolInOrden(&(*p)->izq, n+1, params, accion);
    accion((*p)->info, (*p)->tamInfo, n, params);
    recorrerArbolInOrden(&(*p)->der, n+1, params, accion);
}
