#include "funciones.h"

/* -------------------- hecho en la rama estructura_main previo a la fusion (empieza aca) -------------------- */
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
        if(valorLeido)
        {
            if(*valorLeido == ' ')
                valorLeido += 1;
        }

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
        else //verificar que no sea > 99
        {
            if(strcmp(parametroLeido, "duracion_jornada_minutos") != 0 && atoi(valorLeido) > 99)
            {
                printf("El limite de los parametros es 99 (excepto duracion_jornada_minutos)\n");
                return ERR_PARAM;
            }
            else
            {
                if(strcmp(parametroLeido, "duracion_jornada_minutos") == 0 && atoi(valorLeido) <= 0)
                {
                    printf("La duracion de la jornada debe ser mayor a 0\n");
                    return ERR_PARAM;
                }
            }
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
// modifico esta funcion para poder observar si creo o abrio
int abrirOCrearArchivo(const char* nomArch, FILE **arch, char modo)
{
    if(modo == 't')
    {
        *arch = fopen(nomArch, "r+t");
        if (!*arch)
        {
            *arch = fopen(nomArch, "w+t");
            if(!*arch)
                return ERR_ARCHIVO;
            return ARCH_CREACION;
        }
        return ARCH_APERTURA;
    }
    if(modo == 'b')
    {
        *arch = fopen(nomArch, "r+b");
        if (!*arch)
        {
            *arch = fopen(nomArch, "w+b");
            if(!*arch)
                return ERR_ARCHIVO;
            return ARCH_CREACION;
        }
        return ARCH_APERTURA;
    }

    return MODO_INVALIDO;

}

/// Crear el archivo puerto.txt y llenarlo de forma aleatoria cada pasada
int prepararPuerto(tPuerto* puerto, FILE* archPuerto, const tConfig* configuracion, tCola* contenedores, Cmp cmpInt, Cmp cmpTiempo)
{
    puerto->muelles = NULL;
    puerto->cantMuelles = 0;
    crearLista(&puerto->zonas);
    crearCola(&puerto->buquesEsperando);
    crearCola(&puerto->camionesEsperando);
    crearCola(&puerto->buquesArchivo);
    crearCola(&puerto->camionesArchivo);

    fprintf(archPuerto, "JORNADA:%d\n", configuracion->duracionJornadaMinutos);
    fprintf(archPuerto, "MUELLES:%d\n", configuracion->cantidadMuelles);
    fprintf(archPuerto, "ZONAS:%d\n", configuracion->cantidadZonasAlmacenamiento);
    fprintf(archPuerto, "CAPACIDAD_PILA:%d\n", configuracion->capacidadPila);
    fprintf(archPuerto, "\n");
    fprintf(archPuerto, "[BUQUES]\n");

    // var para errores
    int resultado;

    // Vector de enteros que guarda los tiempos de llegada de cada buque
    int* tiemposBuques = malloc(configuracion->maximoBuques * sizeof(int));
    if(!tiemposBuques)
    {
        printf("Error al asignar memoria para guardar tiempos de llegada de buques\n");
        return ERR_MEMORIA;
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
        return ERR_MEMORIA;
    }

    // Numero de contenedor incluyendo la C
    char nroCont[TAM_NOM_CONTENEDOR];

    // Genera la semilla para el uso de rand() (averiguar para que siempre sea la misma secuencia, para hacer testing)
    srand(time(NULL));
    for(i = 0; i < configuracion->maximoBuques; i++)
    {
        // esto lo que hace es, en la lista de tiempos de llegada de los buques, genera un numero entre 0 y el 90% de la duracion
        // si la duracion es 10, genera un numero entre 0 y 9, para evitar acumulacion de buques al final y tener cierto margen
        // si queremos que tienda mas a cero, se puede hacer con una funcion logaritmica o similar
        *(tiemposBuques + i) = rand() % (int)((configuracion->duracionJornadaMinutos + 1) * 0.9);
    }
    for(i = 0; i < configuracion->maximoCamiones; i++)
    {
        // aca lo mismo pero es el 100% de la duracion el maximo. Aplica a los tiempos de llegada de los camiones
        (camiones + i)->tiempoLlegada = rand() % (int)(configuracion->duracionJornadaMinutos + 1);
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
        snprintf(nroCont, TAM_NOM_CONTENEDOR, "C%d01", b);
        fprintf(archPuerto, "B%03d;T=%d;C=%s", b, *(tiemposBuques + b - 1), nroCont);
        contenedoresTotales++;

        if(ponerEnCola(contenedores, nroCont, TAM_NOM_CONTENEDOR) != TODO_OK)
        {
            printf("Error al asignar memoria para cola de contenedores para puerto.txt\n");
            free(tiemposBuques);
            free(camiones);
            return ERR_MEMORIA;
        }

        // Loop for para registrar los contenedores de cada buque
        // Las condiciones revisan que no haya mas contenedores por buque que los aclarados en config.txt y que no haya mas contenedores que los establecidos en la variable de antes
        // Aca si se suma la variable de contenedores
        for(c = 2; c <= configuracion->maximoContenedoresPorBuque && contenedoresTotales < cantContenedores; c++, contenedoresTotales++)
        {
            // snprintf escribe a la variable del primero parametro (nroCont), la cantidad de bytes del segundo parametro (TAM_NOM_CONTENEDOR), lo que hay en el tercer parametro con formato como un printf normal
            snprintf(nroCont, TAM_NOM_CONTENEDOR, "C%d%02d", b, c);
            fprintf(archPuerto, ",C%d%02d", b, c);
            // Agrego los nombres de los contenedores a una cola para luego desacolar con los camiones
            if(ponerEnCola(contenedores, nroCont, TAM_NOM_CONTENEDOR) != TODO_OK)
            {
                printf("Error al asignar memoria para cola de contenedores para puerto.txt\n");
                free(tiemposBuques);
                free(camiones);
                return ERR_MEMORIA;
            }
        }
        fprintf(archPuerto, "\n");
    }

    // Saco los contenedores de la cola y los guardo en el campo de contenedor de cada camion del vector de camiones
    // (cuando este andando, fijarse si se puede saltear este paso y ponerlo directo donde se agrega a la cola, ahorrando toda la cola en si)
    i = 0;
    while(!colaVacia(contenedores))
    {
        if(sacarDeCola(contenedores, (camiones + i)->codigoContenedor, TAM_NOM_CONTENEDOR) != TODO_OK)
        {
            printf("Error al asignar memoria para sacar de cola\n");
            free(tiemposBuques);
            free(camiones);
            return ERR_MEMORIA;
        }
        i++;
    }
    // Ordenar el vector de camiones por tiempo de llegada
    qsort(camiones, i, sizeof(tCamion), cmpTiempo);

    // Registrar los camiones
    fprintf(archPuerto, "\n");
    fprintf(archPuerto, "[CAMIONES]\n");
    for(k = 1; k <= configuracion->maximoCamiones && k <= i; k++)
    {
        fprintf(archPuerto, "K%03d;T=%d;C=%s\n", k, (camiones + k - 1)->tiempoLlegada, (camiones + k - 1)->codigoContenedor);
    }



    puerto->muelles = calloc(configuracion->cantidadMuelles, sizeof(tMuelle));   // calloc deja buqueAct = NULL
    if(!puerto->muelles){
        free(tiemposBuques);
        free(camiones);
        return ERR_MEMORIA;}
    puerto->cantMuelles = configuracion->cantidadMuelles;
    for(i = 0; i < puerto->cantMuelles; i++)
        sprintf(((puerto->muelles + i)->codMuelle), "M%d", i + 1);
    if(crearZonas(&puerto->zonas, configuracion->cantidadZonasAlmacenamiento, configuracion->capacidadPila) != TODO_OK)
    {
        free(tiemposBuques);
        free(camiones);
        return ERR_MEMORIA;
    }
    fflush(archPuerto);
    rewind(archPuerto);
    resultado = crearBuquesyCamiones(puerto, archPuerto);
    free(tiemposBuques);
    free(camiones);

    return resultado;
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
            puts("Debe ingresar nuevamente\n");
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
            return 'n';
    }
    else
        return 'n';

    // Aca lo mismo, solo que el primer caracter del segundo parametro de la operacion es siempre Z
    if(*param2 == 'Z' && *(param2 + 1) >= '1' && *(param2 + 1) <= '9')
    {
        encontrado = 's';
        if((*(param2 + 2) != ' ' && *(param2 + 2) != '\n' && *(param2 + 2) != '\0') && (*(param2 + 2) < '0' || *(param2 + 2) > '9'))
            return 'n';
    }
    else
        return 'n';

    return encontrado;
}

/// Validar el comando de operacion ingresado y su sintaxis / 'e' = error
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
    if(comando == NULL)
        return 'n';
    char* parametro1;
    char* parametro2;
    char* parametro3;

    // Reviso que sea una operacion posible
    int c;
    char encontrado = 'n';
    for(c = 0; c < CANT_COMANDOS; c++)
    {
        // cambiar indice por aritmetica de punteros?
        if(strcmp(comando, *(comandosValidos+c) ) == 0)
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
        parametro3 = strtok(NULL, " \n\0");
        if(parametro3)
        {
            if(*parametro3 != '\n' && *parametro3 != '\0')
                return 'e';
        }
    }
    else
    {
        parametro1 = strtok(NULL, " \n\0");
        if(parametro1)
        {
            if(*parametro1 != '\n' && *parametro1 != '\0')
                return 'e';
        }
    }

    return encontrado;
}
/* -------------------- hecho en la rama estructura_main previo a la fusion (termina aca) -------------------- */

void printCamion(const void* a)
{
    const tCamion* camion = (const tCamion*)a;
    printf("%s -> solicita %s\n", camion->codigoCamion, camion->codigoContenedor);
}

void printZona(const void* a)
{
    const tZona* zona = (const tZona*)a;
    printf("%s: [", zona->codZona);
    recorrerPila(&zona->contenedores, printCont);
    printf("]\n");
}

void printCont(const void* a)
{
    const tContenedor* cont = (const tContenedor*)a;
    printf("%s ", cont->id);
}

int cmpZona(const void* a, const void* b)
{
    const tZona* z = (const tZona*)a;
    const char* id = (const char*)b;
    return strcmp(z->codZona, id);
}

int cmpContEnZona(const void* a, const void* b)
{
    const tZona* z = (const tZona*)a;
    const char* codCont = (const char*)b;
    tContenedor c;
    if(verTope(&z->contenedores, &c, sizeof(tContenedor)))
        return PILA_VACIA;
    return strcmp(c.id, codCont);
}

int inicializarResumen(tResumen* res)
{
    if(!res)
    {
        printf("\nNo se encontro la variable para resumenes\n");
        return ERR_PARAM;
    }
    res->buquesDescargados = 0;
    res->camionesPendientes = 0;
    res->contenedoresEntregados = 0;
    res->puntuacionFinal = 0;
    res->reorganizaciones = 0;
    res->tiempoUtilizado = 0;
    return TODO_OK;
}

int ejecutarDES( tMuelle *muelle, tZona *zonaDest, tContenedor *contenedor_descarga) // logica de descargar y almacenar
{
    if(muelle->buqueAct == NULL )                       // Verifica si hay ALGO en el muelle q pasa el operario
    {
        return MUELLE_VACIO;
    }

    // if(colaVacia(&((*muelle).buqueAct->cont_pend) ))
    if(pilaVacia(&muelle->buqueAct->cont_pend))           // verifica si hay contenedores pendientes en el buque de ese muelle
        return BUQUE_VACIO;                             // implementado con pila

    if(zonaDest->cantActual >= zonaDest->capacMax)      // verifica que haya espacio en la zona para ingresar contenedor
    {
        return ZONA_SIN_CAPACIDAD;
    }


    //desacolar( &((*muelle).buqueAct->cont_pend), contenedor_descarga, sizeof(tContenedor));
    sacarDePila(&(*muelle).buqueAct->cont_pend, contenedor_descarga, sizeof(tContenedor)); // implementado con pila
    ponerEnPila(&zonaDest->contenedores, contenedor_descarga, sizeof(tContenedor));

    // Contador de zona
    zonaDest->cantActual++;
    if(pilaVacia(&muelle->buqueAct->cont_pend))
        return BUQUE_COMPLETO;

    return TODO_OK;
}

void mostrar_operaciones()
{
    printf("\nOperaciones y comandos disponibles:\n");
    printf("\nDES - Descargar y almacenar.");
    printf("\nREU - Reubicar.");
    printf("\nENT - Entregar.");
    printf("\nVER - Ver estado.");
    printf("\nESP - Esperar.");
    printf("\nPor favor ingrese comando a ejecutar:");

}

int validarCantidad( int leidos, int cant_esperada)
{
    if(leidos < cant_esperada)
    {
        printf("\nFaltan parametros %d.", leidos);
        return FALTAN_PARAMETROS;
    }
    if( leidos > cant_esperada)
    {
        printf("\nCantidad de parametros mayor a la esperada(%d).", cant_esperada);
        return EXCESO_PARAMETROS;
    }
    return TODO_OK;
}

int ejecutarOperacion(char *entradaUsuario, tPuerto *puerto, int* tiempoAct, tConfig* config, tResumen *resumen, FILE **fp)
{
    tOperacion operacion;
    char aux[MAX_CODIGO];
    strcpy(aux, entradaUsuario);
    char *comando = strtok(aux, " \n\0");
    tMuelle *muelleDestino = NULL;
    tZona *zonaDestino = NULL;
    tZona *zonaOrigen = NULL;
    int resultado;

    memset(&operacion, 0, sizeof(tOperacion));

    resultado = detectarOperacion(comando, &zonaOrigen, &zonaDestino, &muelleDestino, &operacion, puerto);
    if(resultado != TODO_OK)
        return resultado;

    operacion.tiempoPorDefault = duracionOperacion(operacion.codOperacion, config);
    operacion.tiempoDeCreacion = *tiempoAct;

    if(*tiempoAct + operacion.tiempoPorDefault > config->duracionJornadaMinutos)
        return TIEMPO_INSUFICIENTE;

    resultado = realizarOperacion(&operacion, zonaOrigen, zonaDestino, muelleDestino, puerto, *tiempoAct, resumen);
    if(resultado != TODO_OK)
        return resultado;

    resultado = realizarEventos(puerto, &operacion, tiempoAct);
    if(resultado != TODO_OK)
        return resultado;


    resumen->puntuacionFinal += operacion.puntuacion;     // realiza suma de puntuación por cada operación finalizada
    operacion.tiempoDeFinalizacion = *tiempoAct;         // registro tiempo de finalizacion de operacion

    escribirJornadaMed(fp, &operacion, comando, zonaOrigen->codZona, zonaDestino->codZona);

    return OP_EXITO;
}

tMuelle* buscarMuelle(tPuerto *puerto, const char *idBuscado)
{
    // para buscar si existe el muelle
    int i=0;
    for (i = 0; i < puerto->cantMuelles; i++)
    {

        if (strcmp(((puerto->muelles + i)->codMuelle), idBuscado) == 0)
        {
            return (puerto->muelles + i);              // Retorna dirección de memoria del muelle
        }
    }
    return NULL;                                       // no encuentra, no esya
}

tZona* buscarZona(tPuerto *puerto, const char *idBuscado)
{
    return buscarLista(&puerto->zonas, idBuscado, cmpZona);
}

int ejecutarREU( tZona* zonaOrigen, tZona* zonaDest, tContenedor* contenedor_desc)
{
    /*
    if( zonaOrigen->contenedores == NULL)
    {
        return ZONA_ORIGEN_VACIA;
    }*/
    if(pilaVacia(&(zonaOrigen->contenedores)))
    {
        return ZONA_ORIGEN_VACIA;
    }

    if( strcmp(zonaOrigen->codZona, zonaDest->codZona) == 0 )
    {
        return ZONAS_IGUALES;
    }

    if( zonaDest->cantActual >= zonaDest->capacMax)
    {
        return ZONA_SIN_CAPACIDAD;
    }

    sacarDePila( &zonaOrigen->contenedores,contenedor_desc,sizeof(tContenedor)); // saco de zona origen
    ponerEnPila(&zonaDest->contenedores, contenedor_desc, sizeof(tContenedor)); // apilo en zona destino


    zonaOrigen->cantActual --;
    zonaDest->cantActual++;
    return TODO_OK;
}

void ejecutarVER( tPuerto* puerto, int tiempoActual, int puntuacionProv)
{
    //tContenedor *prox;
    tContenedor prox;

    printf("\n\t\tESTADO DEL PUERTO\n");
    printf("Tiempo: %d\n", tiempoActual);
    printf("Puntuacion actual: %d\n", puntuacionProv);


    printf("\nMUELLES\n"); // buques atracados en muelle
    int i;
    for(i = 0; i < puerto->cantMuelles; i++)
    {
        // Recorro segun cant muelles
        tMuelle *m = ((puerto->muelles)+i);
        // tMuelle *m = &puerto->muelles[i];
        if( m->buqueAct == NULL )
        {
            printf("Muelle %s: Libre\n", m->codMuelle);
        }
        else
        {
            if(pilaVacia(&(m->buqueAct->cont_pend)))
            {
                printf("%s: %s -> Sin contenedores pendientes.\n", m->codMuelle, m->buqueAct->codigoBuque);
                //printf(" Cod Muelle %s ->", puerto->muelles[i]->codMuelle);
                //printf(" Buque atracado:%s\n", puerto->muelles[i]->buqueAct->codigoBuque);
            }
            else
            {
                //tContenedor *prox = (tContenedor*)m->buqueAct->cont_pend.pri->info;
                verTope(&m->buqueAct->cont_pend, &prox, sizeof(tContenedor));
                printf("%s: %s -> proximo contenedor %s.\n", m->codMuelle, m->buqueAct->codigoBuque, prox.id);
            }
        }
    }

    int contador = 0; // Cant buques esperando
    contador = contarCola(&puerto->buquesEsperando);
    printf("Cantidad de buques esperando muelles disponibles: %d\n", contador);

    // contenido zonas de almacenamiento
    printf("\n --- Zonas de almacenamiento ---\n");
    recorrerLista(&puerto->zonas, printZona);

    printf("\n --- Camiones esperando ---\n");
    recorrerHastaXCola(&puerto->camionesEsperando, printCamion, 3);


}

tZona* buscarZonaTope(tLista zonas, const char *codContBuscado)
{
    return buscarLista(&zonas, codContBuscado, cmpContEnZona);
}

int parsearBuques(tBuque *buque, char *buffer, char* bufferCodigo)
{

    // salgo si no encontro un buque donde debería estar
    char* ptrBuffer = strchr(buffer, 'B');
    if(!ptrBuffer)
    {
        return NO_HALLADO;
    }

    // inicio desarmar buffer
    ptrBuffer = strchr(buffer, '\n');
    if(!ptrBuffer)
    {
        return ERR_LINEA_LARGA;
    }
    *ptrBuffer = '\0';

    while((ptrBuffer = strrchr(buffer, ','))!=NULL) // leo tantos C= como halla en archivo excepto el primero (usa ";")
    {
        strncpy(bufferCodigo, ptrBuffer + 1, TAM_LINEA_CODIGO_CONT);
        *(bufferCodigo+TAM_LINEA_CODIGO_CONT-1) = '\0';
        //bufferCodigo[TAM_LINEA_CODIGO_CONT - 1] = '\0'; // buffer tendra vector de char que ira a pila de codigos de buque actual
        ponerEnPila(&buque->cont_pend, bufferCodigo, strlen(bufferCodigo) + 1); /// PENDIENTE PARA CUANDO PASE TODO A PILA
        *ptrBuffer = '\0';
    }

    ptrBuffer = strrchr(buffer, ';');   // busco el ultimo C= con ";"
    strncpy(bufferCodigo, ptrBuffer + 3, TAM_LINEA_CODIGO_CONT); // ptrBuffer+3 salto ;C= y queda en C de C001
    //bufferCodigo[TAM_LINEA_CODIGO_CONT - 1] = '\0';
    *(bufferCodigo+TAM_LINEA_CODIGO_CONT-1) = '\0';
    ponerEnPila(&buque->cont_pend, bufferCodigo, strlen(bufferCodigo) + 1); // ultima carga al fondo de pila y primera queda en tope
    /// PENDIENTE PARA CUANDO PASE TODO A PILA
    *ptrBuffer = '\0';

    ptrBuffer = strrchr(buffer, ';');
    sscanf(ptrBuffer+3,"%u", &buque->horaLlegada);
    //buque->tiempoLlegada = atoi(ptrBuffer+3);
    *ptrBuffer = '\0';

    strncpy(buque->codigoBuque,buffer,TAM_NOMBRE);
    //buque->codigoBuque[TAM_NOMBRE - 1]='\0';
    *(buque->codigoBuque+TAM_NOMBRE - 1) = '\0';
    // fin desarmar buffer

    return TODO_OK;
}

int parsearCamiones(tCamion *camion, char *buffer)
{
    // salgo si no encontro un camion "K" donde debería estar
    char* ptrBuffer = strchr(buffer, 'K');
    if(!ptrBuffer)
    {
        return NO_HALLADO;
    }

    // inicio desarmar buffer
    ptrBuffer = strchr(buffer, '\n');
    if(!ptrBuffer)
    {
        return ERR_LINEA_LARGA;
    }
    *ptrBuffer = '\0';

    /// Formato esperado: K1;T=0;C=C101
    // solo manejamos un contenedor, pila no necesaria

    ptrBuffer = strrchr(buffer, ';');
    strncpy(camion->codigoContenedor, ptrBuffer+3, TAM_NOM_CONTENEDOR - 1);
    *(camion->codigoContenedor+TAM_NOM_CONTENEDOR-1) = '\0';
    //camion->codigoContenedor[TAM_NOM_CONTENEDOR - 1] = '\0';
    *ptrBuffer = '\0';

    ptrBuffer = strrchr(buffer, ';');
    sscanf(ptrBuffer+3,"%u", &camion->tiempoLlegada);
    *ptrBuffer = '\0';

    strncpy(camion->codigoCamion,buffer,TAM_NOMBRE);
    //camion->codigoCamion[TAM_NOMBRE - 1]='\0';
    *(camion->codigoCamion+TAM_NOMBRE-1) = '\0';
    // fin desarmar buffer

    return TODO_OK;
}

int crearBuquesyCamiones(tPuerto* puerto, FILE *pfPuerto)
{
    tBuque buque;
    tCamion camion;
    int comparacion = 1;

    char buffer[TAM_LINEA_BUFF_LECTURA];
    char bufferCodigo[TAM_LINEA_CODIGO_CONT];

    crearPila(&buque.cont_pend); /// creo pila dinamica para contenedores dentro de buque

    /// Inicio buscando buques
    // leo hasta llegar a [BUQUES]
    while(fgets(buffer, TAM_LINEA_BUFF_LECTURA, pfPuerto) && comparacion != 0)
    {
        comparacion = strncmp(buffer, "[BUQUES]", 8);
    }

    // caso: llego al final del archivo y no hallo BUQUES
    if(comparacion != 0)
    {
        return NO_HALLADO;
    }

    if(parsearBuques(&buque, buffer, bufferCodigo)!=TODO_OK)
    {
        vaciarPila(&buque.cont_pend);
        return NO_HALLADO;
    }

    if(ponerEnCola(&puerto->buquesArchivo, &buque,sizeof(tBuque)) != TODO_OK)
    {
        vaciarPila(&buque.cont_pend);
        return ERR_MEMORIA;
    }

    while(fgets(buffer, TAM_LINEA_BUFF_LECTURA, pfPuerto) && *buffer != '\n')
    {
        crearPila(&buque.cont_pend); // ya estaba creada como tal, pero no la vacio asi no se liberan los nodos
        if(parsearBuques(&buque, buffer, bufferCodigo)!=TODO_OK)
        {
            vaciarPila(&buque.cont_pend);
            return NO_HALLADO;
        }

        if(ponerEnCola(&puerto->buquesArchivo, &buque,sizeof(tBuque)))
        {
            vaciarPila(&buque.cont_pend);
            return ERR_MEMORIA;
        }


    }
    /// avanzo buscando Camiones
    comparacion = 1; // reseteo comparacion

    // primera pasada debo leer hasta llegar a primer camion
    while(fgets(buffer, TAM_LINEA_BUFF_LECTURA, pfPuerto) && comparacion != 0)
    {
        comparacion = strncmp(buffer, "[CAMIONES]", 10);
    }

    // caso: llego al final del archivo y no hallo CAMIONES
    if(comparacion != 0)
        return NO_HALLADO;

    if(parsearCamiones(&camion, buffer)!=TODO_OK)
        return NO_HALLADO;

    if(ponerEnCola(&puerto->camionesArchivo, &camion, sizeof(tCamion)))
        return ERR_MEMORIA;

    while(fgets(buffer, TAM_LINEA_BUFF_LECTURA, pfPuerto))
    {
        if(parsearCamiones(&camion, buffer)!=TODO_OK)
            return NO_HALLADO;

        if(ponerEnCola(&puerto->camionesArchivo, &camion, sizeof(tCamion)))
            return ERR_MEMORIA;
    }

    return TODO_OK;
}

int realizarEventos(tPuerto *puerto, tOperacion *operacion, int *temporizador)
{
    unsigned tiempoFin = *temporizador + operacion->tiempoPorDefault;
    int resultado;

    while(*temporizador < tiempoFin)
    {
        (*temporizador)++;
        if((resultado = vigiladorBuques(puerto, *temporizador))!=TODO_OK)// llegada buques
            return resultado;

        if((resultado = vigiladorCamiones(puerto, *temporizador))!=TODO_OK) // llegada camiones
            return resultado;
    }
    return TODO_OK;
}

int vigiladorBuques(tPuerto *puerto, const int temporizador)
{
    tBuque buque;
    int resultado;
    while(verPrimeroCola(&puerto->buquesArchivo, &buque, sizeof(tBuque)) == TODO_OK && buque.horaLlegada <= temporizador)
    {
        sacarDeCola(&(puerto->buquesArchivo), &buque, sizeof(buque));

        resultado = asignarMuelle(&buque, puerto->muelles, puerto->cantMuelles); // mando a muelle
        if(resultado == PUERTO_LLENO)
            resultado = ponerEnCola(&puerto->buquesEsperando, &buque, sizeof(buque)); //  si no puedo mandar a muelle mando a cola de espera
        if(resultado != TODO_OK)
        {
            vaciarPila(&buque.cont_pend);
            return resultado;
        }
    }
    return TODO_OK;
}

int vigiladorCamiones(tPuerto *puerto, const int temporizador)
{
    tCamion camion;
    int resultado;

    while(verPrimeroCola(&puerto->camionesArchivo, &camion, sizeof(tCamion)) == TODO_OK && camion.tiempoLlegada <= temporizador)
    {
        sacarDeCola(&puerto->camionesArchivo, &camion, sizeof(tCamion));
        resultado = ponerEnCola(&puerto->camionesEsperando, &camion, sizeof(camion));
        if(resultado != TODO_OK)
            return resultado;
    }
    return TODO_OK;
}

int asignarMuelle(tBuque *buque, tMuelle *muelle, int cantMuelles)
{
    int i = 0;
    int encontrado = 0;
    while(i < cantMuelles && !encontrado)
    {
        if((muelle + i)->buqueAct == NULL)
        {
            //asignamos
            (muelle + i)->buqueAct = malloc(sizeof(tBuque));
            if(!(muelle + i)->buqueAct)
                return ERR_MEMORIA;
            *(muelle + i)->buqueAct = *buque;
            encontrado = 1;
        }
        i++;
    }
    if(!encontrado)
        return PUERTO_LLENO;


    return TODO_OK;

}

int detectarOperacion(const char* comando, tZona **zonaOrigen, tZona** zonaDestino, tMuelle **muelleDestino, tOperacion *operacion,
                      tPuerto *puerto)
{
    if (strcmp(comando, "DES") == 0)
    {
        //  (DES, Muelle, Zona)

        char *param1 = strtok(NULL, " "); // Muelle -> M1 // usa NULL porque la vez anterior que se uso fue con aux, es decir, con la copia de la entrada del usuario
        char *param2 = strtok(NULL, " \n\0"); // Zona -> Z1

        //
        *muelleDestino = buscarMuelle(puerto, param1);
        if(*muelleDestino == NULL)
            return MUELLE_INCORRECTO;

        *zonaDestino = buscarZona(puerto, param2);
        if(*zonaDestino == NULL)
            return ZONA_INCORRECTA; // osea zona incorrecta

        operacion->codOperacion = OPERACION_DES;
        return TODO_OK;
    }
    else if (strcmp(comando, "REU") == 0)
    {
        char *param1 = strtok(NULL, " ");
        char *param2 = strtok(NULL, " \n\0");

        // ejecutarREU
        *zonaOrigen = buscarZona(puerto, param1);
        if(*zonaOrigen == NULL)
        {
            return ZONA_INCORRECTA;
        } // osea zona incorrecta

        *zonaDestino = buscarZona(puerto, param2);
        if(*zonaDestino == NULL)
        {
            return ZONA_INCORRECTA;
        }// osea zona incorrecta

        operacion->codOperacion = OPERACION_REU;
        return TODO_OK;
    }
    else if (strcmp(comando, "ESP") == 0)
    {
        operacion->codOperacion = OPERACION_ESP; /// es de fantasia porque
        return TODO_OK;                         /// ya paso tiempo en realizarEventos
    }
    else if(strcmp(comando, "VER") == 0)
    {
        operacion->codOperacion = OPERACION_VER;
        return TODO_OK;
    }

    else if(strcmp(comando, "ENT") == 0)
    {
        operacion->codOperacion = OPERACION_ENT;
        return TODO_OK;
    }
    else
    {
        // El comando ingresado no existe
        printf("Error: Comando '%s' no reconocido.\n", comando);
        return COMANDO_INVALIDO;
    }
    return TODO_OK;
}


int realizarOperacion(tOperacion *operacion, tZona *zonaOrigen, tZona *zonaDestino, tMuelle *muelleDestino,
                      tPuerto *puerto, int tiempoAct, tResumen *resumen)
{
    switch (operacion->codOperacion)
    {
    case OPERACION_DES:
    {
        tContenedor contenedorDescargado; // la aux
        int resultado = ejecutarDES(muelleDestino, zonaDestino, &contenedorDescargado);
        if(resultado != TODO_OK && resultado != BUQUE_COMPLETO){return resultado;}
        printf("Contenedor %s colocado en zona %s\n", contenedorDescargado.id,zonaDestino->codZona);
        if(resultado == BUQUE_COMPLETO) // res == 6 es pq se descargo el ultimo contenedor
        {
            operacion->puntuacion = 5;
            free(muelleDestino->buqueAct);
            muelleDestino->buqueAct = NULL;
            resumen->buquesDescargados++;
            asignarEsperando(puerto);
        }/*
        if( resultado != OP_EXITO)
        {
            printf("Codigo de error: %d.", resultado);
            return resultado;
        }*/
        //printf("Contenedor %s colocado en zona %s\n", contenedorDescargado.id,zonaDestino->codZona);
        strcpy(operacion->codigoContenedor,contenedorDescargado.id);
        return OP_EXITO;
    }


    //break;

    case OPERACION_REU:
    {
        tContenedor contenedor_reubicar;
        int resultado = ejecutarREU(zonaOrigen, zonaDestino, &contenedor_reubicar);
        if(resultado != TODO_OK)
        {
            printf("Error: %d\n", resultado);
            return resultado;
        }
        printf(" El contenedor %s ha sido reubicado en la zona %s desde %s.\n", contenedor_reubicar.id, zonaDestino->codZona, zonaOrigen->codZona);
        resumen->reorganizaciones++;
        strcpy(operacion->codigoContenedor,contenedor_reubicar.id);
    }

    break;

    case OPERACION_VER:
    {
        ejecutarVER(puerto, tiempoAct, resumen->puntuacionFinal);
    }

    break;

    case OPERACION_ENT:
    {
        tContenedor contenedor_entregado;
        int resultado = ejecutarENT(puerto, &contenedor_entregado);              /// REVISAR -> lo que se manda no va a tener sentido, revisar que mandar para que sirva y como procesar lo que hace y devuelve
        if(resultado != TODO_OK)                                                 /// RESPUESTA -> dejamos todo dentro de ejecutarENT, se sacan datos de puerto, se corrije manejo de cola para respetar TDA
            return resultado;
        operacion->puntuacion = 10;
        resumen->contenedoresEntregados++;
        strcpy(operacion->codigoContenedor,contenedor_entregado.id);
        break;
    }

    break;

    case OPERACION_ESP:
    {
        /// REVISAR -> OJO hardcodeado luego pasar a variable de configu
        /// RESPUESTA -> segun consigna consume 1 MINUTO ver pag 8 este es el extracto: Esperar: consume exactamente 1 minuto.
        /// ACTUALIZACION -> no uso ejecutarESP, ya se suma el tiempo en realizarEventos
    }

    break;
    default:
    {
        printf("Operacion no hallada");
    }
    }
    return TODO_OK;
}

int ejecutarENT(tPuerto *puerto, tContenedor *contenedor_entregado)
{
    tCamion camionFrente;

    //if( puerto->camionesEsperando.pri  == NULL )
    if(colaVacia(&(puerto->camionesEsperando)))                                                          // verifico existencia de camiones
        return NO_HAY_CAMIONES;

    //tCamion *camionFrente = (tCamion*)puerto->camionesEsperando.pri->info;                            // contenedor solicitado por camion atendido
    verPrimeroCola(&(puerto->camionesEsperando), &camionFrente, sizeof(tCamion));

    tZona *zonaContenedor = buscarZonaTope(puerto->zonas, camionFrente.codigoContenedor);               // busco zona donde este contenedor ej C101
    if( zonaContenedor == NULL)
    {
        return CONTENEDOR_NO_DISPONIBLE;
    }

    if(sacarDePila(&zonaContenedor->contenedores, camionFrente.codigoContenedor, sizeof(tContenedor)) !=  TODO_OK )
    {
        return ERROR_PILA;
    }
    zonaContenedor->cantActual--;
    strcpy(contenedor_entregado->id,camionFrente.codigoContenedor);                                      // guardo codigo de contenedor                                                                     // le saco un contenedor
    if(sacarDeCola(&puerto->camionesEsperando, &camionFrente, sizeof(tCamion)) != TODO_OK )
    {
        return ERROR_PILA;
    }

    return TODO_OK;
}

int eventoOperadorIni(FILE **fpOperador, FILE **fpJornada, FILE **fpIndice, tOperador *operador, tArbol *arbol, int *nuevoOperador)
{
    int modoAperturaOperador = 0;
    int modoAperturaIndice = 0;
    int modoAperturaJornada = 0;

    modoAperturaOperador = abrirOCrearArchivo("operadores.dat", fpOperador, 'b');
    if(modoAperturaOperador == ERR_ARCHIVO || modoAperturaOperador == MODO_INVALIDO)
    {
        printf("\nError: %d\n", modoAperturaOperador);
        puts("Error al abrir archivo de operadores.\n");
        return ERR_ARCHIVO;
    }
    modoAperturaJornada = abrirOCrearArchivo("jornadas.txt", fpJornada, 't');
    if(modoAperturaJornada == ERR_ARCHIVO || modoAperturaJornada == MODO_INVALIDO)
    {
        puts("Error al abrir archivo de jornadas.\n");
        fclose(*fpOperador);
        return ERR_ARCHIVO;
    }
    modoAperturaIndice = abrirOCrearArchivo("operadores.idx", fpIndice, 'b');
    if(modoAperturaIndice == ERR_ARCHIVO || modoAperturaIndice == MODO_INVALIDO)
    {
        puts("Error al abrir archivo indice.\n");
        fclose(*fpOperador);
        fclose(*fpJornada);
        return ERR_ARCHIVO;
    }

    // se crea operador
    puts("Ingrese el nombre del operador para esta sesion:\n");
    puts("Si su usuario no existe al ingresar un nuevo nombre dispone de 100 caracteres.\n");
    ingresarTexto(TAM_NOMBRE + 1, operador->nombre);

    // cargamos el arbol con archivo operadores
    if(modoAperturaOperador != ARCH_CREACION && modoAperturaIndice != ARCH_CREACION)
    {
        cargarIndiceDesdeArchivo(arbol, *fpIndice);                                       // cargamos indice con operador.idx
        //if(buscarEnArchivoConIndice(*fpIndice, arbol, operador)==CLAVE_NO_ENCONTRADA)// si no encuentra marcamos flag para el final de jornada
        if(buscarEnArchivoConIndice(*fpOperador, arbol, operador)==CLAVE_NO_ENCONTRADA){                                                                               // operador queda con datos de archivo listo para actualizar si encuentra
              *nuevoOperador = 1;                                                        // flag nos hara saber si debemos rearmar indice o no
              operador->puntajeTotal = 0;
        }
    }
    else
    {
        *nuevoOperador = 1;                                                             // por este lado tambien tenemos nuevo operador (primero) + nuevo indice
        operador->puntajeTotal = 0;                                                     // inicializamos su puntaje
    }

    escribirJornadaIni(fpJornada, &modoAperturaJornada);

    return TODO_OK;
}

int eventoOperadorFin(FILE **fpOperador, FILE **fpJornada, FILE **fpIndice, tOperador *operador, tArbol *arbol, int *nuevoOperador, tResumen *resumen, int *error)
{
    if(*nuevoOperador)
    {
        fseek(*fpOperador, 0, SEEK_END);
        fwrite(operador,sizeof(tOperador),1,*fpOperador); // escribimos nuevo operador en archivo directo
        fflush(*fpOperador);
        rewind(*fpOperador);
        indexarArchivoOperador(*fpOperador, arbol);          // cargamos arbol con archivo, esperamos desordenado en mayoria
        guardarIndiceEnArchivoPreOrden(arbol, *fpIndice);    // lo guardamos PreOrden
                                                            // el archivo indice ya queda ordenado
                                                            // para proximas jornadas
    }
    else
    {
        guardarEnArchivoConIndice(*fpOperador, arbol, operador); // guardamos archivo
    }

    escribirJornadaFin(fpJornada, resumen, error, operador);

    return TODO_OK;
}

/**
    Como quedaría jornada guardada en archivo:

    [JORNADA]
    Jornada Nº(numero)
    [HISTORIAL DE JORNADA]
    Tiempo de inicio:(numero)|Tiempo de finalizacion:(numero)|Cod operacion:(cadena char)|Contenedor:(cadena char)|Recurso origen:(cadena char)|Recurso destino:(cadena char) (esto es historial jornada)
    Tiempo de inicio:(numero)|Tiempo de finalizacion:(numero)|Cod operacion:(cadena char)|Contenedor:(cadena char)|Recurso origen:(cadena char)|Recurso destino:(cadena char) (esto es historial jornada)
    Tiempo de inicio:(numero)|Tiempo de finalizacion:(numero)|Cod operacion:(cadena char)|Contenedor:(cadena char)|Recurso origen:(cadena char)|Recurso destino:(cadena char) (esto es historial jornada)
    [ESTADISTICA DE JORNADA]
    "Operador:(cadena char)|Puntuacion final:(numero)|Contenedores entregados:(numero)|Buques descargados:(numero)|Camiones pendientes:(numero)|Movimientos de reorganización realizados:(numero)|Tiempo utilizado:(numero) (esto es resumen jornada)

*/
void escribirJornadaIni(FILE **fp, int *modo)                      // inicia la jornada y iniciamos escritura con num de jornada
{
    unsigned numeroJornada = 0;
    char buffer[TAM_LINEA_BUFF_LECTURA];
    char* ptrBuffer = buffer;

    if(*modo == ARCH_CREACION)                                          // La primera jornada
    {
        fprintf(*fp,"[JORNADA]\n");
        numeroJornada = 1;
        fprintf(*fp,"Jornada Nº%u\n", numeroJornada);
    }
    else                                                                // otras jornadas
    {
        while(fgets(buffer, TAM_LINEA_BUFF_LECTURA, *fp))               // busco numero de jornada y avanzo hasta el final
        {
            if(strncmp(buffer, "Jornada Nº", 10)==0)
               numeroJornada = (unsigned)*((ptrBuffer+10)- '0');
        }
        fprintf(*fp,"[JORNADA]\n");
        fprintf(*fp,"Jornada Nº%u\n", numeroJornada);
    }
}

///   escribirJornadaMed(fp, operacion, comando, zonaOrigen.codZona, zonaDestino.codZona);

void escribirJornadaMed(FILE **fp, const tOperacion *operacion, const char *comando, const char* zonaOrigen, const char* zonaDestino)                           // avanzamos jornada y guardamos operaciones
{

    fprintf(*fp,"[HISTORIAL DE JORNADA]\n");
    fprintf(*fp,"Tiempo de inicio:%u|Tiempo de finalizacion:%u|Cod operacion:%s|Contenedor:%s|Recurso origen:%s|Recurso destino:%s",
            operacion->tiempoDeCreacion, operacion->tiempoDeFinalizacion,
            comando,
            (*(operacion->codigoContenedor) != '0' ? operacion->codigoContenedor : "No Aplica"), // memset lo pone en 0 preguntamos si cambio
            (zonaOrigen ? zonaOrigen : "No Aplica"),
            (zonaDestino ? zonaDestino : "No Aplica"));
}

void escribirJornadaFin(FILE **fp, tResumen *res, int *error, tOperador *o)                       // finalizamos jornada y guardamos estadisticas
{
    if(!*error)
    {
        fprintf(*fp,"[ESTADISTICA DE JORNADA]\n");
        fprintf(*fp, "Operador:%s|Puntuacion final:%u|Contenedores entregados:%u|Buques descargados:%u|Camiones pendientes:%u|Movimientos de reorganización realizados:%u|Tiempo utilizado:%u\n",
                o->nombre,res->puntuacionFinal,res->contenedoresEntregados,res->buquesDescargados,
                res->camionesPendientes, res->reorganizaciones,
                res->tiempoUtilizado);
    }
    else
    {
        fprintf(*fp,"[ERROR EN MEMORIA]\n");
    }
}


int crearZonas(tLista *zonas, unsigned cantZonas, unsigned capacidad)
{
    tZona zona;
    int i;

    crearLista(zonas);

    for(i = 1; i <= cantZonas; i++)
    {
        snprintf(zona.codZona, TAM_COD_ZONA, "Z%u", i);
        crearPila(&zona.contenedores);
        zona.cantActual = 0;
        zona.capacMax = capacidad;

        if(ponerAlFinal(zonas, &zona, sizeof(tZona)) != TODO_OK)
        {
            vaciarLista(zonas);   // las pilas estan vacias
            return ERR_MEMORIA;
        }
    }
    return TODO_OK;
}

int duracionOperacion(int codOperacion, const tConfig *config)
{
    switch(codOperacion)
    {
    case OPERACION_DES:
        return config->tiempoDescargaContenedor;
    case OPERACION_REU:
        return config->tiempoReubicacionContenedor;
    case OPERACION_ENT:
        return config->tiempoCargaCamion;
    case OPERACION_ESP:
        return 1;
    default:
        return 0;
    }
}

void liberarPuerto(tPuerto *puerto)
{
    tBuque buque;
    tZona zona;
    int i;

    // buques que quedaron en un muelle y quiza pila con contenedores
    for(i = 0; i < puerto->cantMuelles; i++)
    {
        if((puerto->muelles + i)->buqueAct)
        {
            vaciarPila(&(puerto->muelles + i)->buqueAct->cont_pend);
            free((puerto->muelles + i)->buqueAct);
        }
    }
    free(puerto->muelles);
    puerto->muelles = NULL;
    puerto->cantMuelles = 0;

    // buques en las dos colas
    while(sacarDeCola(&puerto->buquesArchivo, &buque, sizeof(tBuque)) == TODO_OK)
        vaciarPila(&buque.cont_pend);
    while(sacarDeCola(&puerto->buquesEsperando, &buque, sizeof(tBuque)) == TODO_OK)
        vaciarPila(&buque.cont_pend);

    // camiones
    vaciarCola(&puerto->camionesArchivo);
    vaciarCola(&puerto->camionesEsperando);

    // zonas
    while(sacarPrimeroLista(&puerto->zonas, &zona, sizeof(tZona)) == TODO_OK)
        vaciarPila(&zona.contenedores);
}

void motivo(int codigo)
{
    switch(codigo)
    {
    case COMANDO_INVALIDO:
        printf("\nComando invalido\n");
        break;
    case MUELLE_INCORRECTO:
        printf("\nEl muelle no existe\n");
        break;
    case ZONA_INCORRECTA:
        printf("\nLa zona no existe\n");
        break;
    case MUELLE_VACIO:
        printf("\nNo hay un buque en ese muelle\n");
        break;
    case BUQUE_VACIO:
        printf("\nEl buque no tiene contenedores pendientes\n");
        break;
    case ZONA_SIN_CAPACIDAD:
        printf("\nLa zona de destino esta llena\n");
        break;
    case ZONA_ORIGEN_VACIA:
        printf("\nLa zona de origen esta vacia\n");
        break;
    case ZONAS_IGUALES:
        printf("\nLas zonas deben ser distintas.\n");
        break;
    case NO_HAY_CAMIONES:
        printf("\nNo hay camiones esperando\n");
        break;
    case CONTENEDOR_NO_DISPONIBLE:
        printf("\nEl contenedor no esta en el tope de ninguna zona\n");
        break;
    case TIEMPO_INSUFICIENTE:
        printf("\nNo alcanza el tiempo de jornada para esa operacion\n");
        break;
    default:
        printf("\nOperacion rechazada (codigo %d).\n", codigo);
    }
}

int asignarEsperando(tPuerto* puerto)
{
    tBuque buque;
    while(verPrimeroCola(&puerto->buquesEsperando, &buque, sizeof(tBuque)) == TODO_OK && asignarMuelle(&buque, puerto->muelles, puerto->cantMuelles) == TODO_OK)
    {
        sacarDeCola(&puerto->buquesEsperando, &buque, sizeof(tBuque));
    }

    return TODO_OK;
}

int jornadaTerminada(tPuerto* puerto)
{
    int i;

    if(!colaVacia(&puerto->buquesArchivo)||!colaVacia(&puerto->camionesArchivo)||!colaVacia(&puerto->buquesEsperando)||!colaVacia(&puerto->camionesEsperando))
        return TODO_OK;
    for(i = 0; i < puerto->cantMuelles; i++)
    {
        if((puerto->muelles + i)->buqueAct != NULL)
            return TODO_OK;
    }
    return FIN_JORNADA;
}

int hayOperacionValida(tPuerto* puerto, int tiempoAct, const tConfig* config)
{
    int tiempoRestante = config->duracionJornadaMinutos-tiempoAct;
    int i = 0;
    int hayBuqueEnMuelle = 0;
    int hayREU = 0;
    int hayLugarZona = 0;
    char codZona[TAM_COD_ZONA];
    tZona* zona1;
    tZona* zona2;
    int j = 0;
    tCamion camion;

    while(i < puerto->cantMuelles && !hayBuqueEnMuelle)
    {
        if((puerto->muelles + i)->buqueAct)
            hayBuqueEnMuelle = 1;
        i++;
    }

    i = 1;
    while(i <= config->cantidadZonasAlmacenamiento && !hayREU)
    {
        snprintf(codZona, TAM_COD_ZONA, "Z%d", i);
        zona1 = buscarZona(puerto, codZona);
        if(zona1->cantActual < zona1->capacMax)
            hayLugarZona = 1;

        if(zona1->cantActual > 0)
        {
            j = 1;
            while(j<= config->cantidadZonasAlmacenamiento && !hayREU)
            {
                snprintf(codZona, TAM_COD_ZONA, "Z%d", j);
                zona2 = buscarZona(puerto, codZona);
                if(j!=i && zona2->cantActual < zona2->capacMax)
                {
                    hayREU = 1;
                    hayLugarZona = 1;
                }

                j++;
            }
        }


        i++;
    }

    if(hayBuqueEnMuelle && hayLugarZona && tiempoRestante >= duracionOperacion(OPERACION_DES, config))
        return TODO_OK;

    if(hayREU && tiempoRestante >= duracionOperacion(OPERACION_REU, config))
        return TODO_OK;

    // para ent, tengo que ver si el contenedor que pide el primer camion existe en el tope de alguna zona
    if(tiempoRestante >= duracionOperacion(OPERACION_ENT, config) && verPrimeroCola(&puerto->camionesEsperando, &camion, sizeof(camion)) == TODO_OK && buscarZonaTope(puerto->zonas, camion.codigoContenedor) != NULL)
        return TODO_OK;

    return REVISAR_EVENTOS;
}

int haySkipAProximoEvento(tPuerto* puerto, int* tiempoAct, tConfig* config)
{
    int proximoEvento = 0;
    tBuque buque;
    int hay = 0;
    tCamion camion;
    tOperacion skip;

    if(verPrimeroCola(&puerto->buquesArchivo, &buque, sizeof(tBuque)) == TODO_OK)
    {
        proximoEvento = buque.horaLlegada;
        hay = 1;
    }

    if(verPrimeroCola(&puerto->camionesArchivo, &camion, sizeof(tCamion)) == TODO_OK)
    {
        if(!hay || camion.tiempoLlegada < proximoEvento)
        {
            proximoEvento = camion.tiempoLlegada;
            hay = 1;
        }
    }

    if(!hay)
        return NO_HAY_EVENTOS;

    // :p
    printf("\nTomando un cafe. Avanzando tiempo de %d a %d...\n", *tiempoAct, proximoEvento);

    memset(&skip, 0, sizeof(skip));
    skip.tiempoPorDefault = proximoEvento - *tiempoAct;
    if(realizarEventos(puerto, &skip, tiempoAct) != TODO_OK)
        return ERR_MEMORIA;
    return TODO_OK;
}
