#include <stdio.h>
// #include "tdapila.h"
// #include "tdacola.h"
// #include "tdalista.h"

#define TAM_NOMBRE 100
#define TAM_CONTENEDORES 500
#define TAM_PILA 200
#define TAM_LINEA_BUFF_LECTURA 200
#define TAM_LINEA_CODIGO_CONT 100

#define TODO_OK 0
#define ERR_VIGILADOR 201
#define ERR_ARCHIVO 202
#define ERR_LINEA_LARGA 203
#define ERR_MEMORIA 204

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
    char pila[TAM_PILA];
    unsigned tope;
} tPila;

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

typedef struct
{
    char codigoBuque[TAM_NOMBRE];
    int tiempoLLegada;
    tPila codigoContenedores; // pila dinamica
} tBuque;

typedef struct
{
    char codigoCamion[TAM_NOMBRE];
    int tiempoLLegada;
    char codigoContenedor[TAM_NOMBRE];
} tCamion;

int realizarEventos(tCola *Buques, tCola *Camiones, unsigned *temporizador, FILE *pfLogs, unsigned puntuacion);
int vigiladorBuques(tCola *buques, const unsigned *temporizador, FILE *pfPuerto, unsigned posUltimoBuque);
int vigiladorCamiones(tCola *camiones, const unsigned *temporizador, FILE *pfPuerto, unsigned posUltimoCamion);

int main()
{
    printf("Trabajo Práctico Puerto de Contenedores - Operacion Contrarreloj\n");
    return 0;
}

/*
PROGRAMA "Operación Contrarreloj" -> Borrador

#define puntuacion (para cada operacion)
	(se utiliza para resumen de ejemplo en pag 14)

Main()
{
	crear variables para resumen (total buques, total contenedores entregados, etc)

	aperturas de archivo config.txt.
	seteo de las configuraciones (creacion de temporizador, setea los minutos para cada operacion)

	apertura y/o creacion de logs de operador y jornada (txt distintos)

	apertura y/o creacion de ranking.

	presentación de consola para operario (logueo)

	iniciar sesión de operador (inicio de operaciones / inicio de temporizador)

	apertura y/o creacion de archivo operadores.txt
	agregar operador si no existe

	apertura de archivo puerto.txt.

	creacion de colas camiones y buques

	iniciar_jornada ()	TEMPORIZADOR = 0 M. (revisar si hay buque, camion en tiempo 0) /// TAREA: MATIAS

	while(temporizador < tiempo_disponible_asignado && condiciones_de_cierre_jornada) O while(tiempo > 0 &&  condiciones_de_cierre_jornada)
	{
		presentación de operaciones posibles (en pantalla)

		scanf de operacion (para operador)

		ejecutar_operacion (puede afectar condiciones_de_cierre_jornada, EJ: entregar puede generar Bloqueo operativo)  TEMPORIZADOR = 0 M. | TEMPORIZADOR = 1 M. /// TAREA: LAUTARO

		if - else para operacion o posible switch (ver eficiencia) /// NO TOCAR ESPERAR A PROXIMA REUNION

			(descuenta tiempo)
			(validar comando)
			(validar tiempo disponible con el que más tiempo ocupe de todas las operaciones)
			(validar existan movimientos disponibles (barcos, camion))
			(actualizar logs de jornada, crea porque es una jornada nueva) /// NO TOCAR ESPERAR A PROXIMA REUNION

		realizar eventos (informar/realiza llegada de barcos, camiones, desembarcos) TEMPORIZADOR = 1 M.  | TEMPORIZADOR = 2 M. /// TAREA: NICOLAS
			(informar/realiza operaciones finalizadas)
			(realiza suma de puntuación por cada operación finalizada)
			(actualiza logs de operador, sobreescribe)
			(actualizar variables de resumen)
	}

	restar puntaje por transportes pendientes. /// TAREA: MATIAS

	actualizar ranking. /// TAREA: MATIAS

	cierre de archivos y liberación de memoria dinamica.
}

TODO LO DEMAS NO MARCADO ARIEL

NOTA:
Los lotes de pruebas deben cubrir casos extremos y simples

*/


int realizarEventos(tCola *buques, tCola *camiones, tOperacion *operacion, unsigned *temporizador, FILE *pfOperador, tOperador *operador, unsigned puntuacion, char *nombreArchivoPuerto,
                unsigned posUltimoBuque, unsigned posUltimoCamion)
{

    /// posUltimoBuque y  posUltimoCamion
    // guardo pos de ultimo buque/camion leido en archivo, variable que va en main.

    unsigned tiempoLimite = 0;
    FILE *pfPuerto = fopen(nombreArchivoPuerto,"rt");
    if(!pfPuerto)
    {
        printf("Error al abrir el archivo %s", nombreArchivoPuerto)
        return ERR_ARCHIVO;
    }

    tiempoLimite = *temporizador + operacion->tiempoPorDefault;
    while(temporizador<tiempoLimite)
    {
        if(vigiladorBuques(buques, temporizador, operacion->tiempoPorDefault, pfPuerto, posUltimoBuque)!=TODO_OK)// llegada buques
            {
                fclose(pfPuerto);
                return ERR_VIGILADOR;
            }
        if(vigiladorCamiones(camiones, temporizador, pfPuerto, posUltimoCamion)!=TODO_OK) // llegada camiones
            {
                fclose(pfPuerto);
                return ERR_VIGILADOR;
            }
        temporizador++;
    }

    printf("Finalizada la operación %s" operacion->nombre); // informar/realiza operaciones finalizadas
    puntuacion += operacion->puntuacion; // realiza suma de puntuación por cada operación finalizada
    operacion->tiempoDeFinalizacion = temporizador;

    char buffer[TAM_LINEA_BUFF_LECTURA];
    sprintf(buffer, "Operador: %s - Ranking: %d", operador->nombre, operador->puntajeTotal)
    int cant = strlen(buffer);

    fprintf(pfOperador, "%s", buffer);
    fseek(pfOperador, -cant,SEEK_SET); // retrocedo por si debo actualizar de nuevo en otra iteracion

    /// NUEVA OPERACION A HACER
    // al finalizar toda la jornada y antes de volver a mostrar pantalla llamar a reorganizar puntaje para acomodar al nuevo operador en el archivo de ranking
    // usar variable nuevoOperador para decidir si ejecutar algoritmo de reorganizacion de ranking.

    return TODO_OK;
}

int vigiladorBuques(tCola *buques, const unsigned *temporizador, FILE *pfPuerto, unsigned posUltimoBuque)
{
    tBuque buque;
    if(!crearPila(&buque.codigoContenedores)) /// creo pila dinamica para contenedores dentro de buque
    {
        return ERR_MEMORIA;
    }

    char buffer[TAM_LINEA_BUFF_LECTURA];
    char bufferCodigo[TAM_LINEA_CODIGO_CONT];

    fgets(buffer, 1, pfPuerto+posUltimoBuque); // uso posUltimoBuque para saltar los ya leidos

    // inicio desarmar buffer
    char* ptrBuffer = strchr(buffer, '\n');
    if(!ptrBuffer)
    {
        fclose(pfPuerto);
        return ERR_LINEA_LARGA;
    }
    *ptrBuffer = '\0';

    while(ptrBuffer = strrchr(buffer, ',') // leo tantos C= como halla en archivo excepto el primero (usa ";")
    {
        strncpy(bufferCodigo,ptrBuffer+1,TAM_LINEA_CODIGO_CONT));
        bufferCodigo[TAM_LINEA_CODIGO_CONT]='\0'; // buffer tendra vector de char que ira a pila de codigos de buque actual
        ponerEnPila(buque.codigoContenedores, bufferCodigo, strlen(bufferCodigo));
        *ptrBuffer = '\0';
    }

    ptrBuffer = strrchr(buffer, ';');   // busco el ultimo C= con ";"
    strncpy(bufferCodigo,ptrBuffer+3,TAM_LINEA_CODIGO_CONT));; // ptrBuffer+3 salto ;C= y queda en C de C001
    bufferCodigo[TAM_LINEA_CODIGO_CONT]='\0';
    ponerEnPila(buque.codigoContenedores, bufferCodigo, strlen(bufferCodigo)); // ultima carga al fondo de pila y primera queda en tope
    *ptrBuffer = '\0';

    ptrBuffer = strrchr(buffer, ';');
    sscanf(ptrBuffer+3,"%d", buque.tiempoLlegada);
    *ptrBuffer = '\0';

    strncpy(buque.codigoBuque,buffer,TAM_NOMBRE);
    buque.codigoBuque[TAM_NOMBRE]='\0';
    // fin desarmar buffer

    if(buque.tiempoLLegada==temporizador) // evaluo si buque llego en temporizador actual
    {
        if(asignarMuelle(buque)) // mando a muelle si no puedo mando a cola de espera
        {
            ponerEnCola(buques, buque, sizeof(buque)); // cola de espera
        }
        posUltimoBuque = ftell(pfPuerto); // guardo posicion de ultimo leido en archivo para proxima vuelta
                                        // si no se leyo nada no se guarda ultima pos (evito saltearlo)
    }

    rewind(pfPuerto);
    return TODO_OK;
}

int vigiladorCamiones(tCola *camiones, const unsigned *temporizador, FILE *pfPuerto, unsigned posUltimoCamion)
{
    tCamion camion;

    char buffer[TAM_LINEA_BUFF_LECTURA];

    fgets(buffer, 1, pfPuerto+posUltimoCamion);

    // inicio desarmar buffer
    char* ptrBuffer = strchr(buffer, '\n');
    if(!ptrBuffer)
        {
            return ERR_LINEA_LARGA;
        }
    *ptrBuffer = '\0';

    /// Formato esperado: K1;T=0;C=C101
    // solo manejamos un contenedor, pila no necesaria

    ptrBuffer = strrchr(buffer, ';');
    strncpy(camion->codigoContenedor,ptrBuffer+3,TAM_LINEA_CODIGO_CONT));; // ptrBuffer+3 salto ;C= y queda en C de C001
    *ptrBuffer = '\0';

    ptrBuffer = strrchr(buffer, ';');
    sscanf(ptrBuffer+3,"%d", camion.tiempoLlegada);
    *ptrBuffer = '\0';

    strncpy(buque.codigoBuque,buffer,TAM_NOMBRE);
    camion.codigoCamion[TAM_NOMBRE]='\0';
    // fin desarmar buffer

    if(camion.tiempoLlegada==temporizador) // evaluo si camion llego en temporizador actual
    {
        // solo ponemos en cola para lo demas esperar ACCION de operador
        ponerEnCola(buques, buque, sizeof(buque));
        posUltimoBuque = ftell(pfPuerto); // guardo posicion de ultimo leido en archivo para proxima vuelta
                                        // si no se leyo nada no se guarda ultima pos (evito saltearlo)
    }

    frewind(pfPuerto);
    return TODO_OK;
}

