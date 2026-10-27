#include "primitivas.h"
#include "funciones.h"

int cmpInt(const void* a, const void* b)
{
    return *(const int*)a - *(const int*)b;
}
int cmpTiempo(const void* a, const void* b)
{
    const tCamion* k1 = (const tCamion*)a;
    const tCamion* k2 = (const tCamion*)b;
    return k1->tiempoLlegada - k2->tiempoLlegada;
}

int main()
{
    printf("Trabajo Practico Puerto de Contenedores - Operacion Contrarreloj\n\n\n");

    // Variables para resumen
    tResumen varsResumen;
    int err = inicializarResumen(&varsResumen);
    if(err)
        return err;

    tPuerto puerto;

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
        return ERR_ARCHIVO;
    }
    tConfig configuracion;
    err = leerConfiguracion(archConfig, &configuracion);
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
        return ERR_ARCHIVO;
    }

    // Apertura y/o creacion de archivo de ranking
    FILE* archRanking = abrirOCrearArchivo("ranking.txt");
    if(!archRanking)
    {
        printf("Error al abrir archivo de ranking\n");
        fclose(archJornadas);
        fclose(archOperadores);
        return ERR_ARCHIVO;
    }

    // Creacion y preparacion de archivo puerto.txt
    FILE* archPuerto = fopen("puerto.txt", "wt");
    if(!archPuerto)
    {
        printf("Error al crear archivo puerto.txt\n");
        fclose(archJornadas);
        fclose(archOperadores);
        fclose(archRanking);
        return ERR_ARCHIVO;
    }
    err = prepararPuerto(&puerto, archPuerto, &configuracion, &contenedores, cmpInt, cmpTiempo);
    if(err)
        return err;
    fclose(archPuerto);

    // Inicio de sesion
    tOperador operador;
    printf("\nIngrese su nombre: ");
    ingresarTexto(TAM_NOMBRE + 1, operador.nombre);



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
        return ERR_MEMORIA;
    }

    // Se usa para validar la operacion. char en vez de int para ahorrar espacio (analizar viabilidad de bool)
    // posibles 's' o 'n' (s = si/valida, n = no/invalida)
    char validez;
    int resultadoOperacion = TODO_OK;

    /// While principal
    while(varsResumen.tiempoUtilizado < configuracion.duracionJornadaMinutos && resultadoOperacion != TODO_OK)
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

        resultadoOperacion = ejecutarOperacion(operacion, &puerto, varsResumen.tiempoUtilizado, &(configuracion.duracionJornadaMinutos), varsResumen.puntuacionFinal, &operador);

    }
    printf("\n*\n*\n*\n* Finalizando...\n*\n*\n*\n");

    fclose(archJornadas);
    fclose(archOperadores);
    fclose(archRanking);
    free(operacion);
    vaciarCola(&contenedores);
    return 0;
}
