#ifndef FUNCIONES_H_INCLUDED
#define FUNCIONES_H_INCLUDED

#define TAM_NOMBRE 100
#define TAM_COD_MUELLE 4
#define TAM_PILA 200
#define TAM_NOM_CONTENEDOR 5
#define TAM_COD_ZONA 4
#define CANT_MUELLES
#include <string.h>
#include <stdlib.h>

typedef struct sNodo{
    void*info;
    unsigned tamInfo;
    struct sNodo *sig;
} tNodo;

/*
typedef struct{
    char cola[TAM_COLA];
    unsigned tamDisp;
    unsigned pri;
    unsigned ult;
}tCola;*/


typedef struct{
    tNodo *pri;
    tNodo *ult;
} tCola;
/*
typedef struct{
    char pila[TAM_PILA];
    unsigned tope;
}tPila;
*/
typedef tNodo* tPila;
typedef tNodo* tLista; // a prim nodo

// ---------------------------------

typedef struct{
    char codigoBuque[TAM_NOMBRE];
    tCola cont_pend; // Cola de contenedores pendientes -> Debe ser COLA segun enunciado
    int horaLlegada;
} tBuque;

typedef struct{
    char codMuelle[TAM_COD_MUELLE]; //m1, m2
    tBuque *buqueAct; // buque actual
} tMuelle;

typedef struct{
    char id[TAM_NOM_CONTENEDOR];
} tContenedor;

typedef struct{
    char codZona[TAM_COD_ZONA];
    tPila contenedores;
    unsigned cantActual; // Me dice los contenedores apilados ahora
    unsigned capacMax; // Lo extraigo desde config.txt
} tZona;


typedef struct{
    tLista zonas;
    tMuelle *muelles;
    int cantMuelles; // lo agarro de config.txt
    tCola buquesEsperando;
    tCola camionesEsperando;
}tPuerto; // aca agrupo todo lo que hay AHORA en el puerto.

// ---------------------------------

//int ejecutarDES( tMuelle *muelle, tPila *zonaDest, tContenedor *contenedor_descarga);
int ejecutarDES( tMuelle *muelle, tZona *zonaDest, tContenedor *contenedor_descarga); // ejecuta descarga
void mostrar_operaciones();
int validarCantidad( int leidos, int cant_esperada);
tZona* buscarZona(tPuerto *puerto, const char *idBuscado);
tMuelle* buscarMuelle(tPuerto *puerto, const char *idBuscado) ;



int colaVacia( const tCola *c);
int desacolar( tCola *c, void *info, unsigned tam);
int pilaLlena( const tPila *p, unsigned tam);
int apilar( tPila *p, const void *d, unsigned tamInfo);
#endif // FUNCIONES_H_INCLUDED
