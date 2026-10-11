#ifndef FUNCIONES_H_INCLUDED
#define FUNCIONES_H_INCLUDED

#include "primitivas.h"

typedef struct{
    char codigoBuque[TAM_NOMBRE];
    tPila cont_pend; // Cola de contenedores pendientes -> Debe ser COLA segun enunciado
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
    tCola buquesArchivo;
    tCola camionesArchivo;
}tPuerto; // aca agrupo todo lo que hay AHORA en el puerto.

typedef struct
{
    char nombre[TAM_NOMBRE+1];
    int codOperacion;
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


void mostrar_operaciones();

int validarCantidad( int leidos, int cant_esperada);
tZona* buscarZona(tPuerto *puerto, const char *idBuscado);
tZona* buscarZonaTope(tLista zonas, const char *codContBuscado);
tMuelle* buscarMuelle(tPuerto *puerto, const char *idBuscado) ;

int ejecutarOperacion(char *entradaUsuario, tPuerto *puerto, int tiempoAct, int *tiempoJornada, int puntuacion, tOperador *operador);
int ejecutarDES( tMuelle *muelle, tZona *zonaDest, tContenedor *contenedor_descarga); // ejecuta descarga
int ejecutarENT( tPuerto* puerto);
int ejecutarREU( tZona* zonaOrigen, tZona* zonaDest, tContenedor* contenedor_desc);
void ejecutarVER( tPuerto* puerto, int tiempoActual, int puntuacionProv);

int crearBuquesyCamiones(tPuerto* puerto, FILE *pfPuerto);
int parsearBuques(tBuque *buque, char *buffer, char* bufferCodigo);
int parsearCamiones(tCamion *camion, char *buffer);

int vigiladorBuques(tPuerto *puerto, const int *temporizador, const int *tiempoPorDefault);
int vigiladorCamiones(tPuerto *puerto, const int *temporizador);
int asignarMuelle(tBuque *buque, tMuelle *muelle);

int detectarOperacion(const char* comando, tZona *zonaOrigen, tZona* zonaDestino, tMuelle *muelleDestino, tOperacion *operacion,
                        int *cantPal, char *pal, tPuerto *puerto, int tiempoAct, int puntuacion);
int realizarEventos(tPuerto *puerto, tOperacion *operacion,
                    tOperador *operador, int *temporizador, int *tiempoJornada);
int realizarOperacion(tOperacion *operacion, tZona *zonaOrigen, tZona *zonaDestino, tMuelle *muelleDestino,
                      tPuerto *puerto, int tiempoAct, int puntuacion);


/// Parte Nic (header fin)


#endif // FUNCIONES_H_INCLUDED
