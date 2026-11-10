#ifndef PRIMITIVAS_H_INCLUDED
#define PRIMITIVAS_H_INCLUDED

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#define TAM_NOMBRE 100
#define TAM_COD_MUELLE 4
#define TAM_PILA 200
#define TAM_NOM_CONTENEDOR 6
#define TAM_COD_ZONA 4
#define CANT_MUELLES
#define MAX_LINEA_CONFIG 35
#define MAX_CODIGO 12
#define CANT_COMANDOS 5
#define CANT_COMANDOS_MENU 3

#define TODO_OK 0
#define MUELLE_VACIO 1
#define BUQUE_VACIO 2
#define ZONA_SIN_CAPACIDAD 3
#define ERROR_PILA 4
#define ZONA_INCORRECTA -3
#define MUELLE_INCORRECTO -4
#define PUERTO_LLENO 5
#define BUQUE_COMPLETO 6

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
#define ERR_PARAM 205
#define FIN_JORNADA 206
#define NO_HALLADO 207
#define FIN_CAMIONES 208
#define FIN_BUQUE 209
#define NO_HAY_CAMIONES 210
#define CONTENEDOR_NO_DISPONIBLE 211
#define BLOQUEO_OPERATIVO 212
#define NO_HAY_EVENTOS 213

#define TIEMPO_INSUFICIENTE 300
#define REVISAR_EVENTOS 301

#define ARCH_APERTURA 220
#define ARCH_CREACION 221
#define MODO_INVALIDO 222

#define OPERACION_VER 250
#define OPERACION_DES 251
#define OPERACION_ENT 252
#define OPERACION_REU 253
#define OPERACION_ESP 254
#define INICIAR 255
#define SALIR 256
#define RANKING 257

#define PILA_VACIA 1
#define COLA_VACIA 1
#define LISTA_LLENA 1
#define LISTA_VACIA 2

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

typedef void (*Accion)(const void* a);
typedef int (*Cmp)(const void* a, const void* b);


void crearCola(tCola *p);
int ponerEnCola(tCola *p, const void *d, unsigned cantBytes);
int sacarDeCola(tCola *p, void *d, unsigned cantBytes);
int verPrimeroCola(const tCola *p, void *d, unsigned cantBytes);
int colaVacia( const tCola *c);
void vaciarCola(tCola *p);
int recorrerHastaXCola(const tCola *p, Accion accion, unsigned cant);
int contarCola(tCola *p);

void crearPila(tPila *p);
int ponerEnPila(tPila *p, const void *d, unsigned cantBytes);
int sacarDePila(tPila *p, void *d, unsigned cantBytes);
int pilaLlena( const tPila *p, unsigned tam);
int pilaVacia(const tPila *p);
void vaciarPila(tPila* p);
int recorrerPila(const tPila* p, Accion accion);
int verTope(const tPila* p, void* dato, unsigned cantBytes);

void crearLista(tLista* p);
int listaVacia(const tLista* p);
int listaLlena(const tLista* p, unsigned cantBytes);
void vaciarLista(tLista* p);
int ponerAlComienzo(tLista* p, void* dato, unsigned cantBytes);
int sacarPrimeroLista(tLista* p, void* dato, unsigned cantBytes);
int verPrimeroLista(const tLista* p, void* dato, unsigned cantBytes);
int ponerAlFinal(tLista* p, const void* dato, unsigned cantBytes);
int sacarUltimoLista(tLista* p, void* dato, unsigned cantBytes);
int verUltimoLista(const tLista* p, void* dato, unsigned cantBytes);
int recorrerLista(const tLista* p, Accion accion);
void* buscarLista(const tLista* p, const void* dato, Cmp cmp);

#endif // PRIMITIVAS_H_INCLUDED
