
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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


int leerConfiguracion(FILE* arch, tConfig* configuracion);
FILE* abrirArchivo(char* nomArch);
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

    // config
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

    // Inicio de sesion
    char* nombOperador = malloc(MAX_NOMBRE);
    if(!nombOperador)
    {
        printf("Error al crear variable para el nombre del operador\n");
        fclose(archJornadas);
        fclose(archOperadores);
        fclose(archRanking);
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

        // apertura de archivo puerto.txt.

        // creacion de colas camiones y buques

    char* operacion = malloc(MAX_CODIGO);
    if(!operacion)
    {
        printf("Error al crear variable para la operacion\n");
        fclose(archJornadas);
        fclose(archOperadores);
        fclose(archRanking);
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
    free(nombOperador);
    free(operacion);
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
    char* comando = strtok(aux, " \n");
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
        parametro1 = strtok(NULL, " \n");
        parametro2 = strtok(NULL, " \n");
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
