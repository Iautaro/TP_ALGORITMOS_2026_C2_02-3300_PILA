#ifndef PRIMITIVAS_H_INCLUDED
#define PRIMITIVAS_H_INCLUDED

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#define TAM_NOMBRE 100
#define TAM_COD_MUELLE 4
#define TAM_PILA 200
#define TAM_NOM_CONTENEDOR 5
#define TAM_COD_ZONA 4
#define CANT_MUELLES

#define TODO_OK 0
#define MUELLE_VACIO 1
#define BUQUE_VACIO 2
#define ZONA_SIN_CAPACIDAD 3
#define ERROR_PILA 4
#define ZONA_INCORRECTA -3
#define MUELLE_INCORRECTO -4

#define ERR -1

#define OP_EXITO 0
#define COMANDO_INVALIDO -1
#define FALTAN_PARAMETROS -2
#define PARAMETRO_INCORRECTO -3
#define EXCESO_PARAMETROS -4

#define ZONA_ORIGEN_VACIA -5
#define ZONAS_IGUALES -6

#define MAX_CANT_PAL 4
#define MIN(a,b) ((a) < (b)? (a):(b)) /// DIFERENCIA CON minimo DE MIEL, razón?

#define TAM_NOMBRE 100
#define TAM_CONTENEDORES 500
#define TAM_PILA 200
#define TAM_LINEA_BUFF_LECTURA 200
#define TAM_LINEA_CODIGO_CONT 100
#define TAM_COD_MUELLE 4

#define TODO_OK 0
#define ERR_VIGILADOR 201
#define ERR_ARCHIVO 202
#define ERR_LINEA_LARGA 203
#define ERR_MEMORIA 204
#define FIN_JORNADA 205
#define NO_HALLADO 206
#define FIN_CAMIONES 207
#define FIN_BUQUE 208

#define OPERACION_VER 250
#define OPERACION_DES 251
#define OPERACION_ENT 252
#define OPERACION_REU 253

#define minimo( X , Y ) ( ( X ) <= ( Y ) ? ( X ) : ( Y ) )

typedef struct sNodo{
    void*info;
    unsigned tamInfo;
    struct sNodo *sig;
} tNodo;

typedef struct{
    tNodo *pri;
    tNodo *ult;
} tCola;

typedef tNodo* tPila;
typedef tNodo* tLista; // a prim nodo

void crearCola(tCola *p);
void crearPila(tPila *p);
int ponerEnCola(tCola *p, const void *d, unsigned cantBytes);
int ponerEnPila(tPila *p, const void *d, unsigned cantBytes);
int sacarDeCola(tCola *p, void *d, unsigned cantBytes);
int sacarDePila(tPila *p, void *d, unsigned cantBytes);
int pilaLlena( const tPila *p, unsigned tam);
int pilaVacia(const tPila *p);
int colaVacia( const tCola *c);
int desacolar( tCola *c, void *info, unsigned tam);
int apilar( tPila *p, const void *d, unsigned tamInfo);
int desapilar( tPila *p,  void *d, unsigned tamInfo);

#endif // PRIMITIVAS_H_INCLUDED
