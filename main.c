
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
// #include "tdapila.h"
// #include "tdacola.h"
// #include "tdalista.h"

/// Codigos de retorno
#define TODO_OK 0
#define ERR_ARCH 1
#define SIN_MEM 2
#define ERR_PARAM 3

/// Puntuacion
#define PUNTOS_CONTENEDOR_ENTREGADO 10
#define PUNTOS_BUQUE_DESCARGADO 5
#define PUNTOS_CAMION_PENDIENTE 2

/// Maximos
// Maximo de cada linea del archivo config.txt
#define MAX_LINEA_CONFIG 35
// Maximo de nombre de operador
#define MAX_NOMBRE 51
// Maximo de codigo de operacion a introducir (ej: REU Z1 Z2)
#define MAX_CODIGO 12
// Maximo de numero de contenedor (ej: C001)
#define MAX_NRO_CONT 6
// Cantidad de comandos posibles
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

// Estructura camiones (no incluye codigo de camion)
typedef struct
{
    int tiempo;
    char contenedor[MAX_NRO_CONT];
} tCamion;


/// Puntero a funcion
// Comparacion de enteros y de tiempos de llegada de camiones
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
FILE* abrirOCrearArchivo(char* nomArch);
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

    // Actualmente se guardan en esta cola los contenedores,
    // que luego se van sacando a medida que se les asigna un camion,
    // por lo que al terminar de preparar puerto.txt queda vacia
    tCola contenedores;
    crearCola(&contenedores);

    // Apertura de config.txt y lectura de su contenido a tConfig configuracion, se cierra aca
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

    // Apertura o creacion de archivos de operador y jornada
    FILE* archOperadores = abrirOCrearArchivo("operadores.txt");
    FILE* archJornadas = abrirOCrearArchivo("jornadas.txt");

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
    FILE* archRanking = abrirOCrearArchivo("ranking.txt");
    if(!archRanking)
    {
        printf("Error al abrir archivo de ranking\n");
        fclose(archJornadas);
        fclose(archOperadores);
        return ERR_ARCH;
    }

    // Creacion y preparacion de archivo puerto.txt
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

            iniciar_jornada ()	TEMPORIZADOR = 0 M. (revisar si hay buque, camion en tiempo 0) /// TAREA: MATIAS
        */


    // Creacion de variable para ingresar operacion
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

    // Se usa para validar la operacion. char en vez de int para ahorrar espacio (analizar viabilidad de bool)
    // posibles 's' o 'n' (s = si/valida, n = no/invalida)
    char validez;

    /// While principal
    while(tiempoUtilizado < configuracion.duracionJornadaMinutos && !condicionesCierreJornada)
    {
        // Mostrar los codigos y sus sintaxis
        mostrarOperaciones();

        // Ingreso de operacion
        printf("\nOPERADOR> ");
        ingresarTexto(MAX_CODIGO, operacion);
        // Validacion de la operacion ingresada
        validez = validarOperacion(operacion);

        // Si no es valida, vuelve a pedir y vuelve a evaluar validez
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

/// Lectura del archivo config.txt
int leerConfiguracion(FILE* arch, tConfig* configuracion)
{
    // Variable para la linea que se lee
    char lineaParametro[MAX_LINEA_CONFIG];

    // Parametro de esa linea leida
    char* parametroLeido;
    // Valor de esa linea leida
    char* valorLeido;

    // While hasta que se termine el archivo, lee linea por linea
    while(fgets(lineaParametro, MAX_LINEA_CONFIG, arch))
    {
        // Asegurarse de haber encontrado un \n y reemplazarlo por \0
        char* fin = strchr(lineaParametro, '\n');
        if(!fin)
        {
            printf("Error al obtener linea de parametro\n");
            return ERR_PARAM;
        }
        *fin = '\0';

        // strtok recorre lineaParametro hasta encontrar alguno de los caracteres que hay en el segundo parametro.
        // guarda en parametroLeido el string desde donde arranco hasta el caracter que encontro, el cual reemplaza con \0
        // dentro de la misma funcion strtok se queda guardado un puntero al siguiente caracter del que reemplazo
        // al ponerle NULL continua desde el siguiente del que reemplazo por \0 (es decir no lee el \0)
        // en teoria leeria desde el espacio inclusive, por lo que sumandole uno se lo saltaria
        parametroLeido = strtok(lineaParametro, ":");
        valorLeido = strtok(NULL, "\0:");
        if(*valorLeido == ' ')
            valorLeido += 1;

        // Verificar que todo haya salido bien
        if(!parametroLeido || !valorLeido)
        {
            printf("Error al encontrar parametro en config.txt\n\nPor favor escribir parametro como:\n[NOMBRE_PARAMETRO]: [VALOR]\n");
            return ERR_PARAM;
        }

        // Verificar que no sea negativo
        if(*valorLeido == '-')
        {
            printf("No se pueden ingresar numeros negativos en config.txt\n");
            return ERR_PARAM;
        }

        // Las siguientes comprobaciones sirven para asegurarse que la configuracion se guarda donde se debe
        // (asumiendo que no siempre va a estar en el orden propuesto)
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

/// Apertura del archivo o creacion del archivo si no existe
FILE* abrirOCrearArchivo(char* nomArch)
{
    FILE* arch = fopen(nomArch, "r+t");
    if (!arch)
        arch = fopen(nomArch, "w+t");
    return arch;
}

/// Crear el archivo puerto.txt y llenarlo de forma aleatoria cada pasada
int prepararPuerto(FILE* puerto, const tConfig* configuracion, tCola* contenedores, Cmp cmpInt, Cmp cmpTiempo)
{
    fprintf(puerto, "JORNADA:%d\n", configuracion->duracionJornadaMinutos);
    fprintf(puerto, "MUELLES:%d\n", configuracion->cantidadMuelles);
    fprintf(puerto, "ZONAS:%d\n", configuracion->cantidadZonasAlmacenamiento);
    fprintf(puerto, "CAPACIDAD_PILA:%d\n", configuracion->capacidadPila);
    fprintf(puerto, "\n");
    fprintf(puerto, "[BUQUES]\n");

    // Vector de enteros que guarda los tiempos de llegada de cada buque
    int* tiemposBuques = malloc(configuracion->maximoBuques * sizeof(int));
    if(!tiemposBuques)
    {
        printf("Error al asignar memoria para guardar tiempos de llegada de buques\n");
        return SIN_MEM;
    }

    // Variables para loops
    int i;
    int b;
    int c;
    int k;
    // Contador de contenedores
    int contenedoresTotales = 0;

    // Vector de camiones que guarda los datos de cada camion
    // (menos el numero del camion, que no esta actualmente en la estructura)
    tCamion* camiones = malloc(configuracion->maximoCamiones * sizeof(tCamion));
    if(!camiones)
    {
        printf("Error al asignar memoria para guardar camiones\n");
        free(tiemposBuques);
        return SIN_MEM;
    }

    // Numero de contenedor incluyendo la C
    char nroCont[MAX_NRO_CONT];

    // Genera la semilla para el uso de rand() (averiguar para que siempre sea la misma secuencia, para hacer testing)
    srand(time(NULL));
    for(i = 0; i < configuracion->maximoBuques; i++)
    {
        // esto lo que hace es, en la lista de tiempos de llegada de los buques, genera un numero entre 0 y el 90% de la duracion
        // si la duracion es 10, genera un numero entre 0 y 9, para evitar acumulacion de buques al final y tener cierto margen
        // si queremos que tienda mas a cero, se puede hacer con una funcion logaritmica o similar
        *(tiemposBuques + i*sizeof(int)) = rand() % (int)((configuracion->duracionJornadaMinutos + 1) * 0.9);
    }
    for(i = 0; i < configuracion->maximoCamiones; i++)
    {
        // aca lo mismo pero es el 100% de la duracion el maximo. Aplica a los tiempos de llegada de los camiones
        *(camiones + i * sizeof(camiones)).tiempo = rand() % (int)(configuracion->duracionJornadaMinutos + 1);
    }

    // Ordenamiento de los tiempos de llegada de los buques
    qsort(tiemposBuques, configuracion->maximoBuques, sizeof(int), cmpInt);

    /// Casos hipoteticos
    // Caso 1:
    // imaginemos 2 buques, 3max por buque, 8 camiones.
    // b1 c1 c2 c3, b2 c4 c5 c6; c7 y c8?
    // la cantidad de contenedores no puede ser 8, debe ser 6 (= 2 buques * 3 max por buque)
    // Caso 2:
    // imaginemos 2 buques, 3max por buque, 5 camiones.
    // b1 c1 c2 c3, b2 c1 c2
    // la cantidad de contenedores no puede ser 6 (2*3), debe ser 5 (la cantidad maxima de camiones)
    // Caso 3:
    // imaginemos 2 buques, 3max por buque, 2 camion.
    // b1 c1 c2; b2?
    // la cantidad de contenedores no puede ser 6 (2*3), debe ser 2 (la cantidad maxima de camiones, y un solo buque ya que son menos contenedores que los maximos por buque)

    // Entonces esta variable guarda el min(maximo de camiones, buques * max por buque)
    // Establece la cantidad maxima de contenedores que va a haber, respetando los casos planteados
    int cantContenedores = configuracion->maximoBuques * configuracion->maximoContenedoresPorBuque < configuracion->maximoCamiones ? configuracion->maximoBuques * configuracion->maximoContenedoresPorBuque : configuracion->maximoCamiones;

    // Loop for para registrar los buques con sus contenedores en el archivo
    // Las condiciones revisan que no haya mas buques que los aclarados en config.txt y que no haya mas contenedores que los establecidos en la variable de antes
    // No se suma la variable de los contenedores en cada pasada, solamente se suma al registrar los contenedores (el loop de adentro)
    for(b = 1; b <= configuracion->maximoBuques && contenedoresTotales < cantContenedores; b++)
    {
        fprintf(puerto, "B%03d;T=%d;C=%d01", b, *(tiemposBuques + i), b);

        // Loop for para registrar los contenedores de cada buque
        // Las condiciones revisan que no haya mas contenedores por buque que los aclarados en config.txt y que no haya mas contenedores que los establecidos en la variable de antes
        // Aca si se suma la variable de contenedores
        for(c = 1; c <= configuracion->maximoContenedoresPorBuque && contenedoresTotales < cantContenedores; c++, contenedoresTotales++)
        {
            // snprintf escribe a la variable del primero parametro (nroCont), la cantidad de bytes del segundo parametro (MAX_NRO_CONT), lo que hay en el tercer parametro con formato como un printf normal
            snprintf(nroCont, MAX_NRO_CONT, "C%d%02d", b, c);
            fprintf(puerto, ",C%d%02d", b, c);
            // Agrego los nombres de los contenedores a una cola para luego desacolar con los camiones
            if(ponerEnCola(contenedores, nroCont, strlen(nroCont)) != TODO_OK)
            {
                printf("Error al asignar memoria para cola de contenedores para puerto.txt\n");
                return SIN_MEM;
            }
        }
    }

    // Saco los contenedores de la cola y los guardo en el campo de contenedor de cada camion del vector de camiones
    // (cuando este andando, fijarse si se puede saltear este paso y ponerlo directo donde se agrega a la cola, ahorrando toda la cola en si)
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

    // Ordenar el vector de camiones por tiempo de llegada
    qsort(camiones, configuracion->maximoCamiones, sizeof(int), cmpTiempo);

    // Registrar los camiones
    fprintf(puerto, "\n");
    fprintf(puerto, "\n");
    fprintf(puerto, "[CAMIONES]\n");
    for(k = 1; k <= configuracion->maximoCamiones; k++)
    {
        fprintf(puerto, "K%03d;T=%d;C=%s\n", k, *(camiones + i * sizeof(tCamion)).tiempo, *(camiones + i * sizeof(tCamion)).contenedor);
    }

    free(tiemposBuques);
    free(camiones);
    return TODO_OK;
}

/// Ingresar texto
void ingresarTexto(int max, char* texto)
{
    // Uso fgets en vez de scanf ya que (en teoria) evita que se ingrese un texto mayor al max
    // lo corta con un \0 (entonces si no hay \n significa que lo que ingreso fue muy largo), evitando desborde de memoria
    fgets(texto, max, stdin);
    // fflush por las dudas (SOLO SIRVE PARA WINDOWS, sino hay que usar un while y getchar, ver con ia), ya que el exceso se queda en stdin
    fflush(stdin);

    // Si existe, reemplaza el \n (del enter) en \0, sino debe ingresar nuevamente y vuelve a comprobar
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

/// Mostrar operaciones y su sintaxis
void mostrarOperaciones()
{
    printf("\nDES - Descargar y almacenar: DES <muelle> <zona>");
    printf("\nREU - Reubicar: REU <zona_origen> <zona_destino>");
    printf("\nENT - Entregar: ENT");
    printf("\nVER - Ver estado: VER");
    printf("\nESP - Esperar: ESP");
}

/// Funcion complementaria para validar los parametros dentro de una operacion (se llama desde validarOperacion())
char verificarParametrosOperacion(char* param1, char* param2, char tipoParam1)
{
    // La funcion recibe los dos parametros que se ingresaron y el tipo de parametro 1, que puede ser M si se usa DES o Z si se usa REU. El otro parametro es siempre Z
    char encontrado = 'n';
    // Se verifica que el parametro 1 sea M o Z (segun el parametro de la funcion que se paso), y que el caracter siguiente a la letra este entre 1 y 9 inclusives
    if(*param1 == tipoParam1 && *(param1 + 1) >= '1' && *(param1 + 1) <= '9')
    {
        encontrado = 's';
        // En el caso de que haya 10 o mas muelles o zonas, se revisa que el siguiente caracter al primer numero no sea un espacio (ni \0)
        // Si no lo es, se revisa que sea un numero
        // Entonces, si no es un espacio (lo mas normal) ni es \0 (es decir que hay un digito mas despues del primero numero, ej: Z3X), pero eso que sigue no es un numero, se considera como no encontrado
        // Si hay un digito y dicho digito es un numero, sigue considerandose encontrado
        if((*(param1 + 2) != ' ' && *(param1 + 2) != '\0') && (*(param1 + 2) < '0' || *(param1 + 2) > '9'))
            encontrado = 'n';
    }
    else
        return 'n';

    // Aca lo mismo, solo que el primer caracter del segundo parametro de la operacion es siempre Z
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

/// Validar el comando de operacion ingresado y su sintaxis
char validarOperacion(char* operacion)
{
    const char* comandosValidos[CANT_COMANDOS] = {"DES", "REU", "ENT", "VER", "ESP"};

    // Conversion a mayuscula
    int i;
    for(i = 0; *(operacion + i) != '\0'; i++)
        *(operacion + i) = toupper(*(operacion + i));

    // Uso aux para no perder la operacion original (que se cortaria antes por el \0 que agrega strtok)
    char aux[MAX_CODIGO];
    strcpy(aux, operacion);
    // Primero obtengo el comando, ya que no se si va a tener parametros o no, depende el comando
    char* comando = strtok(aux, " \n\0");
    char* parametro1;
    char* parametro2;

    // Reviso que sea una operacion posible
    int c;
    char encontrado = 'n';
    for(c = 0; c < CANT_COMANDOS; c++)
    {
        // cambiar indice por aritmetica de punteros?
        if(strcmp(comando, comandosValidos[c]) == 0)
            encontrado = 's';
    }

    // Si se encontro, reviso si es alguno de los comandos con parametros, para validarlos
    if(encontrado == 's' && (strcmp(comando, "DES") == 0 || strcmp(comando, "REU") == 0))
    {
        // Ahora si, obtengo los parametros de la operacion con strtok
        parametro1 = strtok(NULL, " \n\0");
        parametro2 = strtok(NULL, " \n\0");
        // Si alguno no se encontro salgo
        if(!parametro1 || !parametro2)
            return 'n';

        // Si se encontro, valido los parametros segun que comando es (si DES o REU)
        if(strcmp(comando, "DES") == 0)
            encontrado = verificarParametrosOperacion(parametro1, parametro2, 'M');
        else
            encontrado = verificarParametrosOperacion(parametro1, parametro2, 'Z');
    }

    return encontrado;
}

void ejecutarOperacion() /// TAREA: LAUTARO
{}

void realizarEventos() /// TAREA: NICOLAS
{}

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

		if - else para operacion o posible switch (ver eficiencia)

			(descuenta tiempo)
			(validar comando)
			(validar tiempo disponible con el que más tiempo ocupe de todas las operaciones)
			(validar existan movimientos disponibles (barcos, camion))
			(actualizar logs de jornada, crea porque es una jornada nueva)

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
