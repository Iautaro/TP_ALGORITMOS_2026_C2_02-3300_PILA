
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
// #include "tdapila.h"
// #include "tdacola.h"
// #include "tdalista.h"

// Codigos de retorno
#define TODO_OK 0
#define ERR_ARCH 1
#define SIN_MEM 2
#define ERR_PARAM 3

// Puntuacion
#define PUNTOS_CONTENEDOR_ENTREGADO 10
#define PUNTOS_BUQUE_DESCARGADO 5
#define PUNTOS_CAMION_PENDIENTE 2

// Maximos
#define MAX_LINEA_CONFIG 35
#define MAX_NOMBRE 51
#define MAX_CODIGO 12
#define MAX_NRO_CONT 6
#define CANT_COMANDOS 5

// Variables de configuracion
typedef struct
{
    unsigned duracionJornadaMinutos;
    unsigned cantidadMuelles;
    unsigned cantidadZonasAlmacenamiento;
    unsigned capacidadPila;
    unsigned maximoBuques;
    unsigned maximoContenedoresPorBuque;
    unsigned maximoCamiones;
    unsigned tiempoDescargaContenedor;
    unsigned tiempoReubicacionContenedor;
    unsigned tiempoCargaCamion;
} tConfig;

// Estructura camiones
typedef struct
{
    int tiempo;
    char contenedor[MAX_NRO_CONT];
} tCamion;


// Puntero a funcion
typedef int (*Cmp)(const void* a, const void* b);
int cmpInt(const void* a, const void* b)
{
    return *(const int*)a - *(const int*)b;
}
int cmpTiempo(const void* a, const void* b)
{
    const tCamion* k1 = (const tCamion*)a;
    const tCamion* k2 = (const tCamion*)b;
    return k1->tiempo - k2->tiempo;
}


int leerConfiguracion(FILE* arch, tConfig* configuracion);
FILE* abrirArchivo(char* nomArch);
int prepararPuerto(FILE* puerto, const tConfig* configuracion, tCola* contenedores, Cmp cmpInt, Cmp cmpTiempo);
void ingresarTexto(int max, char* texto);
void mostrarOperaciones();
char verificarParametrosOperacion(char* param1, char* param2, char tipoParam1);
char validarOperacion(char* operacion);
void ejecutarOperacion(); /// TAREA: LAUTARO
void realizarEventos(); /// TAREA: NICOLAS




int main() {
    printf("Trabajo Práctico Puerto de Contenedores - Operacion Contrarreloj\n\n\n");

    // Variables para resumen
    unsigned contenedoresEntregados = 0;
    unsigned buquesDescargados = 0;
    unsigned camionesPendientes = 0;
    unsigned reorganizaciones = 0;
    unsigned tiempoUtilizado = 0;
    unsigned puntuacionFinal = 0;

    tCola contenedores;

    crearCola(&contenedores);

    // Config
    FILE* archConfig = fopen("config.txt", "rt");
    if (!archConfig)
    {
        printf("Error al abrir config.txt\n");
        return ERR_ARCH;
    }

    tConfig configuracion;
    int err = leerConfiguracion(archConfig, &configuracion);
    fclose(archConfig);
    if (err)
        return err;

    // Apertura y/o creacion de archivos de operador y jornada
    FILE* archOperadores = abrirArchivo("operadores.txt");
    FILE* archJornadas = abrirArchivo("jornadas.txt");

    if (!archOperadores || !archJornadas)
    {
        printf("Error al abrir archivo de ");
        if (!archOperadores && !archJornadas)
            printf("operadores y jornadas\n");
        else
        {
            if (!archOperadores)
            {
                printf("operadores\n");
                fclose(archJornadas);
            }
            else
            {
                printf("jornadas\n");
                fclose(archOperadores);
            }
        }
        return ERR_ARCH;
    }

    // Apertura y/o creacion de archivo de ranking
    FILE* archRanking = abrirArchivo("ranking.txt");
    if(!archRanking)
    {
        printf("Error al abrir archivo de ranking\n");
        fclose(archJornadas);
        fclose(archOperadores);
        return ERR_ARCH;
    }

    FILE* archPuerto = fopen("puerto.txt", "wt");
    if(!archPuerto)
    {
        printf("Error al crear archivo puerto.txt\n");
        fclose(archJornadas);
        fclose(archOperadores);
        fclose(archRanking);
        return ERR_ARCH;
    }
    err = prepararPuerto(archPuerto, &configuracion, contenedores, cmpInt, cmpTiempo);
    if(err)
        return err;


    // Inicio de sesion
    char* nombOperador = malloc(MAX_NOMBRE);
    if(!nombOperador)
    {
        printf("Error al crear variable para el nombre del operador\n");
        fclose(archJornadas);
        fclose(archOperadores);
        fclose(archRanking);
        fclose(archPuerto);
        return SIN_MEM;
    }

    printf("\nIngrese su nombre: ");
    ingresarTexto(MAX_NOMBRE, nombOperador);

    /// PENDIENTES
        /*
            iniciar sesión de operador (inicio de operaciones / inicio de temporizador)     /// ESPERAR A ARBOLES

            apertura y/o creacion de archivo operadores.txt                                 /// ESPERAR A ARBOLES
            agregar operador si no existe
        */



    char* operacion = malloc(MAX_CODIGO);
    if(!operacion)
    {
        printf("Error al crear variable para la operacion\n");
        fclose(archJornadas);
        fclose(archOperadores);
        fclose(archRanking);
        fclose(archPuerto);
        free(nombOperador);
        return SIN_MEM;
    }

    // se usa para validar la operacion. char en vez de int para ahorrar espacio. posibles 's' o 'n' (s = si/valida, n = no/invalida)
    char validez;

    while(tiempoUtilizado < configuracion.duracionJornadaMinutos && !condicionesCierreJornada)
    {
        mostrarOperaciones();

        printf("\nOPERADOR> ");
        ingresarTexto(MAX_CODIGO, operacion);
        validez = validarOperacion(operacion);

        while(validez == 'n')
        {
            printf("\nOperacion invalida.");
            printf("\nOPERADOR> ");
            ingresarTexto(MAX_CODIGO, operacion);
            validez = validarOperacion(operacion);
        }

        ejecutarOperacion(); /// TAREA: LAUTARO

        realizarEventos(); /// TAREA: NICOLAS
    }


    fclose(archJornadas);
    fclose(archOperadores);
    fclose(archRanking);
    fclose(archPuerto);
    free(nombOperador);
    free(operacion);
    vaciarCola(contenedores);
    return 0;
}

int leerConfiguracion(FILE* arch, tConfig* configuracion)
{
    char lineaParametro[MAX_LINEA_CONFIG];
    char* parametroLeido;
    char* valorLeido;

    while(fgets(lineaParametro, MAX_LINEA_CONFIG, arch))
    {
        char* fin = strchr(lineaParametro, '\n');
        if(!fin)
        {
            printf("Error al obtener linea de parametro\n");
            return ERR_PARAM;
        }
        *fin = '\0';

        parametroLeido = strtok(lineaParametro, ":");
        valorLeido = strtok(NULL, ":");

        if(!parametroLeido || !valorLeido)
        {
            printf("Error al encontrar parametro en config.txt\n\nPor favor escribir parametro como:\n[NOMBRE_PARAMETRO]: [VALOR]\n");
            return ERR_PARAM;
        }
        if(*valorLeido == '-')
        {
            printf("No se pueden ingresar numeros negativos en config.txt\n");
            return ERR_PARAM;
        }

        if(strcmp(parametroLeido, "duracion_jornada_minutos") == 0)
        {
            configuracion->duracionJornadaMinutos = atoi(valorLeido);
        }
        else if(strcmp(parametroLeido, "cantidad_muelles") == 0)
        {
            configuracion->cantidadMuelles = atoi(valorLeido);
        }
        else if(strcmp(parametroLeido, "cantidad_zonas_almacenamiento") == 0)
        {
            configuracion->cantidadZonasAlmacenamiento = atoi(valorLeido);
        }
        else if(strcmp(parametroLeido, "capacidad_pila") == 0)
        {
            configuracion->capacidadPila = atoi(valorLeido);
        }
        else if(strcmp(parametroLeido, "maximo_buques") == 0)
        {
            configuracion->maximoBuques = atoi(valorLeido);
        }
        else if(strcmp(parametroLeido, "maximo_contenedores_por_buque") == 0)
        {
            configuracion->maximoContenedoresPorBuque = atoi(valorLeido);
        }
        else if(strcmp(parametroLeido, "maximo_camiones") == 0)
        {
            configuracion->maximoCamiones = atoi(valorLeido);
        }
        else if(strcmp(parametroLeido, "tiempo_descarga_contenedor") == 0)
        {
            configuracion->tiempoDescargaContenedor = atoi(valorLeido);
        }
        else if(strcmp(parametroLeido, "tiempo_reubicacion_contenedor") == 0)
        {
            configuracion->tiempoReubicacionContenedor = atoi(valorLeido);
        }
        else if(strcmp(parametroLeido, "tiempo_carga_camion") == 0)
        {
            configuracion->tiempoCargaCamion = atoi(valorLeido);
        }
        else
        {
            printf("No se reconocio el siguiente parametro: %s\n", parametroLeido);
        }
    }
    return TODO_OK;
}

FILE* abrirArchivo(char* nomArch)
{
    FILE* arch = fopen(nomArch, "r+t");
    if (!arch)
        arch = fopen(nomArch, "w+t");
    return arch;
}

int prepararPuerto(FILE* puerto, const tConfig* configuracion, tCola* contenedores, Cmp cmpInt, Cmp cmpTiempo)
{
    fprintf(puerto, "JORNADA:%d\n", configuracion->duracionJornadaMinutos);
    fprintf(puerto, "MUELLES:%d\n", configuracion->cantidadMuelles);
    fprintf(puerto, "ZONAS:%d\n", configuracion->cantidadZonasAlmacenamiento);
    fprintf(puerto, "CAPACIDAD_PILA:%d\n", configuracion->capacidadPila);
    fprintf(puerto, "\n");
    fprintf(puerto, "[BUQUES]\n");

    int* tiemposBuques = malloc(configuracion->maximoBuques * sizeof(int));
    if(!tiemposBuques)
    {
        printf("Error al asignar memoria para guardar tiempos de buques\n");
        return SIN_MEM;
    }

    int i;
    int b;
    int c;
    int k;
    int contenedoresTotales = 0;

    tCamion* camiones = malloc(configuracion->maximoCamiones * sizeof(tCamion));
    if(!cammiones)
    {
        printf("Error al asignar memoria para guardar camiones\n");
        free(tiemposBuques);
        return SIN_MEM;
    }




    char nroCont[MAX_NRO_CONT];

    srand(time(NULL));
    for(i = 0; i < configuracion->maximoBuques; i++)
    {
        // esto lo que hace es, en la lista de tiempos de llegada, genera un numero entre 0 y el 90% de la duracion
        // si la duracion es 10, genera un numero entre 0 y 9, para evitar acumulacion de buques al final y tener cierto margen
        // si queremos que tienda mas a cero, se puede hacer con una funcion logaritmica o similar
        *(tiemposBuques + i*sizeof(int)) = rand() % (int)((configuracion->duracionJornadaMinutos + 1) * 0.9);
    }
    for(i = 0; i < configuracion->maximoCamiones; i++)
    {
        // aca lo mismo pero es el 100% de la duracion el maximo
        *(camiones + i * sizeof(camiones)).tiempo = rand() % (int)(configuracion->duracionJornadaMinutos + 1);
    }

    qsort(tiemposBuques, configuracion->maximoBuques, sizeof(int), cmpInt);

    // imaginemos 2 buques, 3max por buque, 8 camiones.
    // b1 c1 c2 c3, b2 c4 c5 c6; c7 y c8?
    // imaginemos 2 buques, 3max por buque, 2 camion.
    // b1 c1 c2; b2?
    // imaginemos 2 buques, 3max por buque, 5 camiones.
    // b1 c1 c2 c3, b2 c1 c2

    int cantContenedores = configuracion->maximoBuques * configuracion->maximoContenedoresPorBuque < configuracion->maximoCamiones ? configuracion->maximoBuques * configuracion->maximoContenedoresPorBuque : configuracion->maximoCamiones;

    for(b = 1; b <= configuracion->maximoBuques && contenedoresTotales < cantContenedores; b++)
    {
        fprintf(puerto, "B%03d;T=%d;C=%d01", b, *(tiemposBuques + i), b);
        for(c = 1; c <= configuracion->maximoContenedoresPorBuque && contenedoresTotales < cantContenedores; c++, contenedoresTotales++)
        {
            // esta funcion escribe a la variable nroCont
            snprintf(nroCont, MAX_NRO_CONT, "C%d%02d", b, c);
            fprintf(puerto, ",C%d%02d", b, c);
            if(ponerEnCola(contenedores, nroCont, strlen(nroCont)) != TODO_OK)
            {
                printf("Error al asignar memoria para cola de contenedores para puerto.txt\n");
                return SIN_MEM;
            }
        }
    }

    // antes de ordenar por tiempo, desacolo los contenedores y genero los tiempos, luego ordeno por tiempo y los contenedores quedan mezclados

    i = 0;
    while(!colaVacia(contenedores))
    {
        if(sacarDeCola(contenedores, *(camiones + i * sizeof(tCamion)).contenedor, MAX_NRO_CONT) != TODO_OK)
        {
            printf("Error al asignar memoria para sacar de cola\n");
            return SIN_MEM;
        }
        i++;
    }

    qsort(camiones, configuracion->maximoCamiones, sizeof(int), cmpTiempo);



    fprintf(puerto, "\n");
    fprintf(puerto, "\n");
    fprintf(puerto, "[CAMIONES]\n");
    for(k = 1; k <= configuracion->maximoCamiones; k++)
    {
        fprintf(puerto, "K%03d;T=%d;C=%s\n", k, *(camiones + i * sizeof(tCamion)).tiempo, *(camiones + i * sizeof(tCamion)).contenedor);
    }




    // T < duracionJornadaMinutos
    // B = maximoBuques
    // C = maximoContenedoresPorBuque
    // K = maximoCamiones
    free(tiempos);
    return TODO_OK;
}

void ingresarTexto(int max, char* texto)
{
    fgets(texto, max, stdin);
    fflush(stdin);

    char* fin = strchr(texto, '\n');
    if(fin && fin != texto)
        *fin = '\0';
    else
    {
        while(fin == texto || fin == NULL)
        {
            printf("\nIngrese nuevamente: ");
            fgets(texto, max, stdin);
            fflush(stdin);

            fin = strchr(texto, '\n');
            if(fin && fin != texto)
                *fin = '\0';
        }
    }
}

void mostrarOperaciones()
{
    printf("\nDES - Descargar y almacenar: DES <muelle> <zona>");
    printf("\nREU - Reubicar: REU <zona_origen> <zona_destino>");
    printf("\nENT - Entregar: ENT");
    printf("\nVER - Ver estado: VER");
    printf("\nESP - Esperar: ESP");
}

char verificarParametrosOperacion(char* param1, char* param2, char tipoParam1)
{
    char encontrado = 'n';
    if(*param1 == tipoParam1 && *(param1 + 1) >= '1' && *(param1 + 1) <= '9')
    {
        encontrado = 's';
        if((*(param1 + 2) != ' ' && *(param1 + 2) != '\0') && (*(param1 + 2) < '0' || *(param1 + 2) > '9'))
            encontrado = 'n';
    }
    else
        return 'n';
    if(*param2 == 'Z' && *(param2 + 1) >= '1' && *(param2 + 1) <= '9')
    {
        encontrado = 's';
        if((*(param2 + 2) != ' ' && *(param2 + 2) != '\n' && *(param2 + 2) != '\0') && (*(param2 + 2) < '0' || *(param2 + 2) > '9'))
            encontrado = 'n';
    }
    else
        return 'n';

    return encontrado;
}

char validarOperacion(char* operacion)
{
    const char* comandosValidos[CANT_COMANDOS] = {"DES", "REU", "ENT", "VER", "ESP"};

    // Conversion a mayuscula
    int i;
    for(i = 0; *(operacion + i) != '\0'; i++)
        *(operacion + i) = toupper(*(operacion + i));

    char aux[MAX_CODIGO];
    strcpy(aux, operacion);
    char* comando = strtok(aux, " \n\0");
    char* parametro1;
    char* parametro2;

    int c;
    char encontrado = 'n';
    for(c = 0; c < CANT_COMANDOS; c++)
    {
        if(strcmp(comando, comandosValidos[c]) == 0 )
            encontrado = 's'; // Comando existe
    }
    if(encontrado == 's' && (strcmp(comando, "DES") == 0 || strcmp(comando, "REU") == 0))
    {
        parametro1 = strtok(NULL, " \n\0");
        parametro2 = strtok(NULL, " \n\0");
        if(!parametro1 || !parametro2)
            return 'n';
        if(strcmp(comando, "DES") == 0)
            encontrado = verificarParametrosOperacion(parametro1, parametro2, 'M');
        else
            encontrado = verificarParametrosOperacion(parametro1, parametro2, 'Z');
    }

    return encontrado; // Comando erroneo
}

void ejecutarOperacion() /// TAREA: LAUTARO
{}

void realizarEventos() /// TAREA: NICOLAS
{}

/*
PROGRAMA "Operación Contrarreloj" -> Borrador

#define puntuacion (para cada operacion)                /// HECHO
	(se utiliza para resumen de ejemplo en pag 14)

Main()
{
	crear variables para resumen (total buques, total contenedores entregados, etc)     /// HECHO

	aperturas de archivo config.txt.                                                                    /// HECHO
	seteo de las configuraciones (creacion de temporizador, setea los minutos para cada operacion)      /// HECHO

	apertura y/o creacion de logs de operador y jornada (txt distintos)         /// HECHO

	apertura y/o creacion de ranking.           /// HECHO

	presentación de consola para operario (logueo)      /// HECHO

	iniciar sesión de operador (inicio de operaciones / inicio de temporizador)     /// ESPERAR A ARBOLES

	apertura y/o creacion de archivo operadores.txt             /// ESPERAR A ARBOLES
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
