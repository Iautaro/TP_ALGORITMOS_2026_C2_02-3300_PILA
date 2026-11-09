#include "primitivas.h"



void crearPila(tPila *p)
{
    *p = NULL;
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

void vaciarPila(tPila* p)
{
    while(*p)
    {
        tNodo* aux = *p;
        *p = aux->sig;
        free(aux->info);
        free(aux);
    }
}

void crearCola(tCola *p)
{
    p->pri = NULL;
    p->ult = NULL;
}

int ponerEnCola(tCola *p, const void *d, unsigned cantBytes)
{
    tNodo* nue = (tNodo*)malloc(sizeof(tNodo));
    if(!nue)
        return ERR_MEMORIA;
    nue->info = malloc(cantBytes);
    if(!nue->info)
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

int verPrimeroCola(const tCola *p, void *d, unsigned cantBytes)
{
    if(p->pri == NULL)
        return COLA_VACIA;
    memcpy(d, p->pri->info, minimo(cantBytes, p->pri->tamInfo));
    return TODO_OK;
}

int sacarDeCola(tCola *p, void *d, unsigned cantBytes)
{
    if(p->pri == NULL || p->ult == NULL)
        return COLA_VACIA;
    tNodo *aux = p->pri;
    p->pri = aux->sig;
    memcpy(d, aux->info, minimo(aux->tamInfo, cantBytes));
    free(aux->info);
    free(aux);
    if(p->pri == NULL)
        p->ult = NULL;
    return TODO_OK;
}

int colaVacia(const tCola *c)
{
    return c->pri == NULL || c->ult == NULL;
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


int recorrerPila(const tPila* p, Accion accion)
{
    tNodo *aux = *p;
    if(aux == NULL)
        return PILA_VACIA;
    while(aux)
    {
        accion(aux->info);
        aux = aux->sig;
    }
    return TODO_OK;
}

int verTope(const tPila* p, void* dato, unsigned cantBytes)
{
    if(*p == NULL)
        return PILA_VACIA;
    memcpy(dato, (*p)->info, cantBytes < (*p)->tamInfo ? cantBytes : (*p)->tamInfo);
    return TODO_OK;
}

int recorrerHastaXCola(const tCola *p, Accion accion, unsigned cant)
{
    tNodo *aux = p->pri;
    int posicion = 0;

    if(aux == NULL)
        return COLA_VACIA;

    while(aux != NULL && posicion < cant)
    {
        accion(aux->info);
        aux = aux->sig;
        posicion++;
    }
    return TODO_OK;
}

int contarCola(tCola *p)
{
    tNodo* aux = p->pri;
    int cont = 0;
    while(aux)
    {
        cont++;
        aux = aux->sig;
    }
    return cont;
}

void crearLista(tLista* p)
{
    *p = NULL;
}

int listaVacia(const tLista* p)
{
    return *p == NULL;
}

int listaLlena(const tLista* p, unsigned cantBytes)
{
    tNodo* aux = (tNodo*)malloc(sizeof(tNodo));
    if(!aux)
        return LISTA_LLENA;
    aux->info = malloc(cantBytes);
    if(!aux->info)
    {
        free(aux);
        return LISTA_LLENA;
    }
    free(aux->info);
    free(aux);
    return TODO_OK;
}

void vaciarLista(tLista* p)
{
    while(*p != NULL)
    {
        tNodo* aux = *p;
        *p = aux->sig;
        free(aux->info);
        free(aux);
    }
}

int ponerAlComienzo(tLista* p, void* dato, unsigned cantBytes)
{
    tNodo* nue = (tNodo*)malloc(sizeof(tNodo));
    if(!nue)
        return LISTA_LLENA;
    nue->info = malloc(cantBytes);
    if(!nue->info)
    {
        free(nue);
        return LISTA_LLENA;
    }
    memcpy(nue->info, dato, cantBytes);
    nue->tamInfo = cantBytes;
    nue->sig = *p;
    *p = nue;
    return TODO_OK;
}

int sacarPrimeroLista(tLista* p, void* dato, unsigned cantBytes)
{
    if(*p == NULL)
        return LISTA_VACIA;
    tNodo* aux = *p;
    *p = aux->sig;
    memcpy(dato, aux->info, cantBytes < aux->tamInfo ? cantBytes : aux->tamInfo);
    free(aux->info);
    free(aux);
    return TODO_OK;
}

int verPrimeroLista(const tLista* p, void* dato, unsigned cantBytes)
{
    if(*p == NULL)
        return LISTA_VACIA;
    memcpy(dato, (*p)->info, cantBytes < (*p)->tamInfo ? cantBytes : (*p)->tamInfo);
    return TODO_OK;
}

int ponerAlFinal(tLista* p, const void* dato, unsigned cantBytes)
{
    tNodo* nue = (tNodo*)malloc(sizeof(tNodo));
    if(!nue)
        return LISTA_LLENA;
    nue->info = malloc(cantBytes);
    if(!nue->info)
    {
        free(nue);
        return LISTA_LLENA;
    }
    while(*p)
        p = &(*p)->sig;

    memcpy(nue->info, dato, cantBytes);
    nue->tamInfo = cantBytes;
    nue->sig = NULL;
    *p = nue;
    return TODO_OK;
}

int sacarUltimoLista(tLista* p, void* dato, unsigned cantBytes)
{
    if(*p == NULL)
        return LISTA_VACIA;
    while((*p)->sig)
        p = &(*p)->sig;
    memcpy(dato, (*p)->info, cantBytes < (*p)->tamInfo ? cantBytes : (*p)->tamInfo);
    free((*p)->info);
    free(*p);
    *p = NULL;
    return TODO_OK;
}

int verUltimoLista(const tLista* p, void* dato, unsigned cantBytes)
{
    if(*p == NULL)
        return LISTA_VACIA;
    while((*p)->sig)
        p = &(*p)->sig;
    memcpy(dato, (*p)->info, (*p)->tamInfo < cantBytes ? (*p)->tamInfo : cantBytes);
    return TODO_OK;
}

int recorrerLista(const tLista* p, Accion accion)
{
    tNodo *aux = *p;
    if(aux == NULL)
        return LISTA_VACIA;
    while(aux)
    {
        accion(aux->info);
        aux = aux->sig;
    }
    return TODO_OK;
}

void* buscarLista(const tLista* p, const void* dato, Cmp cmp)
{
    int encontrado = 0;
    tNodo* aux = *p;
    while(!encontrado && aux)
    {
        if(cmp(aux->info, dato) == 0)
            encontrado = 1;
        if(!encontrado)
            aux = aux->sig;
    }
    if(!encontrado)
        return NULL;
    return aux->info;
}
