#include "primitivas.h"
#include "funciones.h"

int main()
{
    int err = 0;
    int nuevoOperador = 0;
    int nroJornada = 0;
    int resultadoEleccion = 0;
    int eleccion = 0;
    int i;
    FILE* archConfig = NULL;
    FILE* archPuerto = NULL;
    FILE* archOperadores = NULL;
    FILE* archJornadas = NULL;
    FILE* archIndice = NULL;
    tArbol arbol;
    tOperador operador;
    tContenedor proxCont;
    tResumen varsResumen;
    tPuerto puerto;
    tConfig configuracion;
    char* operacion;

    // Se usa para validar la operacion. char en vez de int para ahorrar espacio
    // posibles 's' o 'n' (s = si/valida, n = no/invalida) o 'e' = error
    char validez;

    int resultadoOperacion = TODO_OK;
    int tiempoActual = 0;
    int skipearTiempo;
    int desdeTiempo = -1; // -1 cuando no se skipea tiempo, si es mayor o igual a 0 significa que se skipeo
    int fatalError = 0;
    int finJornada = TODO_OK; // 0(TODO_OK) si no es el fin, admite TODO_OK, FIN_JORNADA como fin normal, BLOQUEO_OPERATIVO como bloqueo operativo, y OPERACION_SAL cuando el jugador decide cortar la jornada


    printf("Trabajo Practico Puerto de Contenedores - Operacion Contrarreloj\n\n\n");

    // Variables para resumen
    err = inicializarResumen(&varsResumen);
    if(err != TODO_OK)
        return err;


    // Actualmente se guardan en esta cola los contenedores,
    // que luego se van sacando a medida que se les asigna un camion,
    // por lo que al terminar de preparar puerto.txt queda vacia

    // Apertura de config.txt y lectura de su contenido a tConfig configuracion, se cierra aca
    archConfig = fopen("config.txt", "rt");
    if (!archConfig)
    {
        printf("Error al abrir config.txt\n");
        return ERR_ARCHIVO;
    }
    memset(&configuracion, 0, sizeof(tConfig));
    err = leerConfiguracion(archConfig, &configuracion);
    fclose(archConfig);
    if (err != TODO_OK)
    {
        printf("Error al leer las configuraciones\n");
        return err;
    }

    // Creacion y preparacion de archivo puerto.txt
    archPuerto = fopen("puerto.txt", "w+t");
    if(!archPuerto)
    {
        printf("Error al crear archivo puerto.txt\n");
        return ERR_ARCHIVO;
    }
    err = prepararPuerto(&puerto, archPuerto, &configuracion, cmpInt, cmpTiempo);
    if(err != TODO_OK)
    {
        fclose(archPuerto);
        liberarPuerto(&puerto);
        return err;
    }
    fclose(archPuerto);
    /// RESPUESTA(20) -> se crea evento operador inicial y final para manejar arboles y sesion
    /// IMPORTANTE(20) -> el indice tiene mismos campos que el operador, nos queda doble clave para el indice, se puede reducir? o ampliamos
    ///                   caracteristicas de operador para que no quede igual indice y archivo?
    /// reducimos clave a solo codigo
    // Apertura o creacion de archivos de operador y jornada, tambien es inicio de sesion
    memset(&operador, 0, sizeof(tOperador));
    crearArbol(&arbol);
    if((err = eventoOperadorIni(&archOperadores, &archJornadas, &archIndice, &operador, &arbol, &nuevoOperador, &nroJornada)) != TODO_OK)
    {
        liberarPuerto(&puerto);
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
    operacion = malloc(MAX_CODIGO);
    if(!operacion)
    {
        printf("Error al crear variable para la operacion\n");
        fclose(archJornadas);
        fclose(archOperadores);
        fclose(archIndice);
        liberarPuerto(&puerto);
        eliminarArbol(&arbol);
        return ERR_MEMORIA;
    }


    // por si hay alguno en T=0
    if(vigiladorBuques(&puerto, 0) != TODO_OK || vigiladorCamiones(&puerto, 0) != TODO_OK)
        fatalError = 1;
    else
        asignarEsperando(&puerto);

    /// Menu principal
    mostrarMenu();
    printf("\n\nOPERADOR> ");
    while(eleccion != SALIR && eleccion != INICIAR && !fatalError)
    {
        ingresarTexto(MAX_CODIGO, operacion);
        validez = validarEleccion(operacion);
        while(validez != 's')
        {
            printf("\nEleccion invalida.");
            printf("\n\nOPERADOR> ");
            ingresarTexto(MAX_CODIGO, operacion);
            validez = validarEleccion(operacion);
        }
        resultadoEleccion = ejecutarEleccion(operacion, &archOperadores, &eleccion);
        if(resultadoEleccion == ERR_MEMORIA)
            fatalError = 1;
        else
        {
            if(resultadoEleccion!= TODO_OK)
                motivo(resultadoEleccion);
        }
    }
    if(eleccion == SALIR)
    {
        fflush(archJornadas);
        fprintf(archJornadas,"Jornada sin eventos - Salida por Menu\n");
        eliminarArbol(&arbol);
        fclose(archJornadas);
        fclose(archOperadores);
        fclose(archIndice);
        free(operacion);
        liberarPuerto(&puerto);
        return TODO_OK;
    }


    /// While principal de jornada
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
                if(desdeTiempo < 0)
                    desdeTiempo = tiempoActual;
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
                if(desdeTiempo >= 0)
                {
                    printf("\nTomando un cafe. Avanzando tiempo de %d a %d...\n", desdeTiempo, tiempoActual);
                    desdeTiempo = -1;
                }

                // mostrar el tiempo y los muelles
                // si un muelle esta libre, se muestra libre
                // si un muelle tiene buque, se muestra el proximo contenedor del buque
                printf("\nT=%d\n", tiempoActual);
                for(i = 0; i < puerto.cantMuelles; i++)
                {
                    if((puerto.muelles + i)->buqueAct == NULL)
                        printf("%s: Libre\n", (puerto.muelles + i)->codMuelle);
                    else
                    {
                        if(verTope(&(puerto.muelles + i)->buqueAct->cont_pend, &proxCont, sizeof(tContenedor)) == TODO_OK)
                            printf("%s: %s -> proximo contenedor %s\n", (puerto.muelles + i)->codMuelle, (puerto.muelles + i)->buqueAct->codigoBuque, proxCont.id);
                    }

                }

                // Mostrar los codigos y sus sintaxis
                mostrarOperaciones();

                // Ingreso de operacion
                printf("\n\nOPERADOR> ");
                ingresarTexto(MAX_CODIGO, operacion);
                // Validacion de la operacion ingresada
                validez = validarOperacion(operacion);

                // Si no es valida, vuelve a pedir y vuelve a evaluar validez
                while(validez != 's')
                {
                    printf("\nOperacion invalida.");
                    printf("\n\nOPERADOR> ");
                    ingresarTexto(MAX_CODIGO, operacion);
                    validez = validarOperacion(operacion);
                }

                resultadoOperacion = ejecutarOperacion(operacion, &puerto, &tiempoActual, &configuracion, &varsResumen, &archJornadas);
                if(resultadoOperacion == ERR_MEMORIA)
                    fatalError = 1;
                else if(resultadoOperacion == OPERACION_SAL)
                {
                    finJornada = OPERACION_SAL;
                }
                else
                {
                    if(resultadoOperacion!= TODO_OK)
                        motivo(resultadoOperacion);
                }
            }
        }
    }

    if(finJornada == OPERACION_SAL)
        printf("\n\nFue un gusto, nosotros nos encargamos desde aca.\n");
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
        varsResumen.puntuacionFinal -= PENALIZACION*varsResumen.camionesPendientes;
        operador.puntajeTotal += varsResumen.puntuacionFinal;
    }
    printf("\n*\n*\n*\n* Cerrando numeros...\n*\n*\n*\n");

    operador.jornadasTotal++;
    eventoOperadorFin(&archOperadores, &archJornadas, &archIndice, &operador, &arbol, &nuevoOperador, &varsResumen, &fatalError);

    fflush(archJornadas);
    rewind(archJornadas);
    leerJornada(archJornadas, nroJornada);

    eliminarArbol(&arbol);
    fclose(archJornadas);
    fclose(archOperadores);
    fclose(archIndice);
    free(operacion);
    liberarPuerto(&puerto);
    return TODO_OK;
}
