#ifndef FUNCIONES_H_INCLUDED
#define FUNCIONES_H_INCLUDED

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

typedef struct
{
    char nombre[TAM_NOMBRE+1];
    int tiempoPorDefault;
    int tiempoDeCreacion;
    int tiempoDeFinalizacion;
    int contenedor;
    int recurso;
    int puntuacion;
} tOperacion;

typedef struct
{
    char nombre[TAM_NOMBRE+1];
    int puntajeTotal;
} tOperador;

typedef struct sNodo
{
    void *info;
    unsigned tamInfo;
    struct sNodo *sig;
} tNodo;

typedef struct
{
    tNodo *pri,
          *ult;
} tCola;

typedef tNodo *tPila;
typedef tNodo* tLista; // a prim nodo

typedef struct
{
    char codigoBuque[TAM_NOMBRE];
    int tiempoLlegada;
    tPila codigoContenedores; // pila dinamica
} tBuque;

typedef struct
{
    char codigoCamion[TAM_NOMBRE];
    int tiempoLlegada;
    char codigoContenedor[TAM_NOMBRE];
} tCamion;

typedef struct{
    char codMuelle[TAM_COD_MUELLE]; //m1, m2
    tBuque *buqueAct; // buque actual
} tMuelle;

typedef struct{
    tLista zonas;
    tMuelle *muelles;
    int cantMuelles;
    tCola buquesEsperando;
    tCola camionesEsperando;
}tPuerto;

int realizarEventos(tCola *buques, tCola *camiones, tMuelle *muelle, tOperacion *operacion, unsigned *temporizador, FILE *pfOperador, tOperador *operador,
                char *nombreArchivoPuerto, unsigned posUltimoBuque, unsigned posUltimoCamion, unsigned *tiempoJornada);
int vigiladorBuques(tCola *buques, tMuelle *muelle, const unsigned *temporizador, const int *tiempoPorDefault, FILE *pfPuerto, unsigned posUltimoBuque);
int vigiladorCamiones(tCola *camiones, const unsigned *temporizador, FILE *pfPuerto, unsigned posUltimoCamion);
int asignarMuelle(tBuque *buque, tMuelle *muelle);

/**
-- CODIGO PROVISIONAL SOLO PARA TEST --
*/

void crearPila(tPila *p);
int ponerEnPila(tPila *p, const void *d, unsigned cantBytes);
int ponerEnCola(tCola *p, const void *d, unsigned cantBytes);

#endif // FUNCIONES_H_INCLUDED
