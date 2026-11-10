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
    int err = 0;
    int nuevoOperador = 0;
    FILE* archOperadores = NULL;
    FILE* archJornadas = NULL;
    FILE* archIndice = NULL;
    FILE* archRanking = NULL;
    tOperador operador;
    tArbol arbol;
    printf("Trabajo Practico Puerto de Contenedores - Operacion Contrarreloj\n\n\n");

    // Variables para resumen
    tResumen varsResumen;
    err = inicializarResumen(&varsResumen);
    if(err != TODO_OK)
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
    if (err != TODO_OK)
    {
        printf("Error al leer las configuraciones\n");
        return err;
    }

    // Apertura y/o creacion de archivo de ranking
    err = abrirOCrearArchivo("ranking.txt", &archRanking, 't');
    if(err == ERR_ARCHIVO || err == MODO_INVALIDO)
    {
        printf("Error al abrir archivo de ranking\n");
        return err;
    }
    else
    {
        if(err == ARCH_CREACION)
            printf("ranking.txt creado exitosamente\n");
    }

    // Creacion y preparacion de archivo puerto.txt
    FILE* archPuerto = fopen("puerto.txt", "w+t");
    if(!archPuerto)
    {
        printf("Error al crear archivo puerto.txt\n");
        fclose(archRanking);
        return ERR_ARCHIVO;
    }
    err = prepararPuerto(&puerto, archPuerto, &configuracion, &contenedores, cmpInt, cmpTiempo);
    if(err != TODO_OK)
    {
        fclose(archRanking);
        fclose(archPuerto);
        liberarPuerto(&puerto);
        vaciarCola(&contenedores);
        return err;
    }
    fclose(archPuerto);
    /// RESPUESTA(20) -> se crea evento operador inicial y final para manejar arboles y sesion
    /// IMPORTANTE(20) -> el indice tiene mismos campos que el operador, nos queda doble clave para el indice, se puede reducir? o ampliamos
    ///                   caracteristicas de operador para que no quede igual indice y archivo?
    /// reducimos clave a solo codigo
    // Apertura o creacion de archivos de operador y jornada, tambien es inicio de sesion
    crearArbol(&arbol);
    if((err = eventoOperadorIni(&archOperadores, &archJornadas, &archIndice, &operador, &arbol, &nuevoOperador)) != TODO_OK)
    {
        fclose(archRanking);
        liberarPuerto(&puerto);
        vaciarCola(&contenedores);
        eliminarArbol(&arbol);
        printf("Falla al realizar el evento de operador con error %d.\n", err);
        return err;
    }

    /// REVISAR(20) -> esto esta pendiente, el 20 es random pero esta linkeado a RESPUESTA(20)
        /*
            iniciar sesión de operador (inicio de operaciones / inicio de temporizador)
            apertura y/o creacion de archivo operadores.txt
            agregar operador si no existe
            iniciar_jornada ()	TEMPORIZADOR = 0 M. (revisar si hay buque, camion en tiempo 0)
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
    // posibles 's' o 'n' (s = si/valida, n = no/invalida) o 'e' = error
    char validez;
    int resultadoOperacion = TODO_OK;
    int tiempoActual = 0;
    int skipearTiempo;
    int fatalError = 0;
    int finJornada = TODO_OK; // 0(TODO_OK) si no es el fin, admite TODO_OK, FIN_JORNADA como fin normal y BLOQUEO_OPERATIVO como bloqueo operativo

    // por si hay alguno en T=0
    if(vigiladorBuques(&puerto, 0) != TODO_OK || vigiladorCamiones(&puerto, 0) != TODO_OK)
        fatalError = 1;
    else
        asignarEsperando(&puerto);


    /// While principal
    // son varios if anidados para evitar que se ejecute codigo en caso de errores, ya que no puedo cortar el while
    while(tiempoActual < configuracion.duracionJornadaMinutos && !fatalError && finJornada == TODO_OK)
    {
        // Chequear que la jornada pueda arrancar/seguir
        finJornada = jornadaTerminada(&puerto);
        if(finJornada != TODO_OK)
            printf("\nNo queda nada por hacer\n");
        else
        {
            if(hayOperacionValida(&puerto, tiempoActual, &configuracion) == REVISAR_EVENTOS)
            {
                skipearTiempo = haySkipAProximoEvento(&puerto, &tiempoActual, &configuracion);
                if(skipearTiempo == ERR_MEMORIA)
                    fatalError = 1;
                else
                {
                    if(skipearTiempo == NO_HAY_EVENTOS)
                        finJornada = BLOQUEO_OPERATIVO; // se genera bloqueo porque no hay operacion valida ni proximo evento
                }
            }
            else
            {
                // Mostrar los codigos y sus sintaxis
                mostrarOperaciones();

                // Ingreso de operacion
                printf("\nOPERADOR> ");
                ingresarTexto(MAX_CODIGO, operacion);
                // Validacion de la operacion ingresada
                validez = validarOperacion(operacion);

                // Si no es valida, vuelve a pedir y vuelve a evaluar validez
                while(validez != 's')
                {
                    printf("\nOperacion invalida.");
                    printf("\nOPERADOR> ");
                    ingresarTexto(MAX_CODIGO, operacion);
                    validez = validarOperacion(operacion);
                }

                resultadoOperacion = ejecutarOperacion(operacion, &puerto, &tiempoActual, &configuracion, &varsResumen, &archJornadas);
                if(resultadoOperacion == ERR_MEMORIA)
                    fatalError = 1;
                else
                {
                    if(resultadoOperacion!= TODO_OK)
                        motivo(resultadoOperacion);
                }
            }
        }
    }

    if(finJornada == BLOQUEO_OPERATIVO)
        printf("\n\nOcurrio un bloqueo operativo. Lo vamos a hablar con tu jefe.\n");
    if(finJornada == TODO_OK && !fatalError)
        printf("\n\nYa podes irte a casa, la jornada finalizo.\n");

    /// RESPUESTA(20) -> se crea evento operador inicial y final para manejar arboles y sesion
    /// REVISAR(21) -> pendiente los cierres del operador y fichar los eventos de la jornada en archivo jornada
    // Aca van ultimos pasos para registrar jornada y cerrar operador.
    if(fatalError)
        printf("\nError de memoria!\n");
    else
    {
        varsResumen.tiempoUtilizado = tiempoActual;
        varsResumen.camionesPendientes = contarCola(&puerto.camionesEsperando);
        varsResumen.puntuacionFinal -= 2*varsResumen.camionesPendientes;
        operador.puntajeTotal += varsResumen.puntuacionFinal;
    }
    printf("\n*\n*\n*\n* Cerrando numeros...\n*\n*\n*\n");
    eventoOperadorFin(&archOperadores, &archJornadas, &archIndice, &operador, &arbol, &nuevoOperador, &varsResumen, &fatalError);


    eliminarArbol(&arbol);
    fclose(archJornadas);
    fclose(archOperadores);
    fclose(archRanking);
    fclose(archIndice);
    free(operacion);
    vaciarCola(&contenedores);
    liberarPuerto(&puerto);
    return 0;
}
