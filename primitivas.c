#include "primitivas.h"

void crearCola(tCola *p)
{
    p->pri = NULL;
    p->ult = NULL;
}

void crearPila(tPila *p)
{
    *p = NULL;
}

int ponerEnCola(tCola *p, const void *d, unsigned cantBytes)
{
    tNodo *nue = (tNodo *) malloc(sizeof(tNodo));

    if(nue == NULL || (nue->info = malloc(cantBytes)) == NULL)
    {
        free(nue);
        return ERR_MEMORIA;
    }

    memcpy(nue->info, d, cantBytes);
    nue->tamInfo = cantBytes;
    nue->sig = NULL;

    if(p->ult)
        p->ult->sig = nue;
    else
        p->pri = nue;

    p->ult = nue;

    return TODO_OK;
}

int ponerEnPila(tPila *p, const void *d, unsigned cantBytes)
{
    tNodo *nue;

    if((nue = (tNodo *)malloc(sizeof(tNodo))) == NULL ||
            (nue->info = malloc(cantBytes)) == NULL)
    {
        free(nue);
        return ERR_MEMORIA;
    }

    memcpy(nue->info, d, cantBytes);
    nue->tamInfo = cantBytes;
    nue->sig = *p;
    *p = nue;

    return TODO_OK;
}

int sacarDeCola(tCola *p, void *d, unsigned cantBytes)
{
    tNodo *aux = p->pri;
    if(aux == NULL)
        return 1;
    p->pri = aux->sig;
    memcpy(d, aux->info, minimo(aux->tamInfo, cantBytes));
    free(aux->info);
    free(aux);
    if(p->pri == NULL)
        p->ult = NULL;
    return TODO_OK;
}

int sacarDePila(tPila *p, void *d, unsigned cantBytes)
{
    tNodo *aux = *p;

    if(aux == NULL)
        return 1;
    *p = aux->sig;
    memcpy(d, aux->info, minimo(cantBytes, aux->tamInfo));
    free(aux->info);
    free(aux);
    return TODO_OK;
}

int pilaVacia(const tPila *p)
{
        return *p == NULL;
}

int colaVacia(const tCola *c)
{
    return c->pri == NULL;
}

int desacolar( tCola *c, void *info, unsigned tam)
{
    if( c->pri == NULL)
        return 1; // error

    tNodo *elim = c->pri;
    c->pri = elim->sig;

    if( c->pri == NULL)
    {
        c->ult = NULL;
    }

    memcpy(info,elim->info,MIN(tam,elim->tamInfo) );
    free( elim->info);
    free(elim);

    return TODO_OK;
}

int apilar( tPila *p, const void *d, unsigned tamInfo)
{
    tNodo *nuevoNodo = malloc( sizeof(tNodo));
    if( nuevoNodo == NULL)
        return 1; // fallo
    nuevoNodo->info = malloc(tamInfo);
    if( nuevoNodo->info == NULL)
    {
        free(nuevoNodo);
        return 1; // fallo
    }
    memcpy( nuevoNodo->info, d,tamInfo);
    nuevoNodo->tamInfo = tamInfo;

    nuevoNodo->sig = *p;
    *p = nuevoNodo;
    return TODO_OK;
}

void vaciarCola(tCola *p)
{
    while(p->pri)
    {
        tNodo* aux = p->pri;
        p->pri = aux->sig;
        free(aux->info);
        free(aux);
    }
    p->ult = NULL;
}


