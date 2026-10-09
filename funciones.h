#ifndef FUNCIONES_H_INCLUDED
#define FUNCIONES_H_INCLUDED

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
// #include "tdapila.h"
// #include "tdacola.h"
// #include "tdalista.h"

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

/// Parte Nic (header inicio)

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

#define minimo( X , Y ) ( ( X ) <= ( Y ) ? ( X ) : ( Y ) )

/// Parte Nic (header fin)

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

/// Parte Nic (header inicio)
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

typedef struct
{
    char codigoCamion[TAM_NOMBRE];
    int tiempoLlegada;
    char codigoContenedor[TAM_NOMBRE];
} tCamion;
/// Parte Nic (header fin)

// ---------------------------------

int ejecutarDES( tMuelle *muelle, tZona *zonaDest, tContenedor *contenedor_descarga); // ejecuta descarga
void mostrar_operaciones();
int validarCantidad( int leidos, int cant_esperada);
tZona* buscarZona(tPuerto *puerto, const char *idBuscado);
tZona* buscarZonaTope(tLista zonas, const char *codContBuscado);
tMuelle* buscarMuelle(tPuerto *puerto, const char *idBuscado) ;

int ejecutarENT( tPuerto* puerto);
int ejecutarREU( tZona* zonaOrigen, tZona* zonaDest, tContenedor* contenedor_desc);
void ejecutarVER( tPuerto* puerto, int tiempoActual, int puntuacionProv);
int ejecutarOperacion(char *entradaUsuario, tPuerto *puerto, int tiempoAct, int puntuacion);

int colaVacia( const tCola *c);
int desacolar( tCola *c, void *info, unsigned tam);
int pilaLlena( const tPila *p, unsigned tam);
int apilar( tPila *p, const void *d, unsigned tamInfo);
int desapilar( tPila *p,  void *d, unsigned tamInfo);

/// Parte Nic (header inicio)
int parsearBuques(tBuque *buque, char *buffer, char* bufferCodigo);
int parsearCamiones(tCamion *camion, char *buffer);
int crearBuquesyCamiones(tCola *buquesArchivo, tCola *camionesArchivo, FILE *pfPuerto);

int realizarEventos(tCola *buquesEsperando, tCola *camionesEsperando, tCola *buquesArchivo, tCola *camionesArchivo, tMuelle *muelle, tOperacion *operacion,
                    tOperador *operador, unsigned *temporizador, unsigned *tiempoJornada);

int vigiladorBuques(tCola *buquesEsperando, tCola *buquesArchivo, tMuelle *muelle, const unsigned *temporizador, const int *tiempoPorDefault);
int vigiladorCamiones(tCola *camionesEsperando, tCola *camionesArchivo, const unsigned *temporizador);

int asignarMuelle(tBuque *buque, tMuelle *muelle);

void crearCola(tCola *p);
void crearPila(tPila *p);
int ponerEnCola(tCola *p, const void *d, unsigned cantBytes);
int ponerEnPila(tPila *p, const void *d, unsigned cantBytes);

/// Parte Nic (header fin)


#endif // FUNCIONES_H_INCLUDED
