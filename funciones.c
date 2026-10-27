#include "funciones.h"
#include "primitivas.h"

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
int prepararPuerto(tPuerto* puerto, FILE* archPuerto, const tConfig* configuracion, tCola* contenedores, Cmp cmpInt, Cmp cmpTiempo)
{
    fprintf(archPuerto, "JORNADA:%d\n", configuracion->duracionJornadaMinutos);
    fprintf(archPuerto, "MUELLES:%d\n", configuracion->cantidadMuelles);
    fprintf(archPuerto, "ZONAS:%d\n", configuracion->cantidadZonasAlmacenamiento);
    fprintf(archPuerto, "CAPACIDAD_PILA:%d\n", configuracion->capacidadPila);
    fprintf(archPuerto, "\n");
    fprintf(archPuerto, "[BUQUES]\n");

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
        *(tiemposBuques + i*sizeof(int)) = rand() % (int)((configuracion->duracionJornadaMinutos + 1) * 0.9);
    }
    for(i = 0; i < configuracion->maximoCamiones; i++)
    {
        // aca lo mismo pero es el 100% de la duracion el maximo. Aplica a los tiempos de llegada de los camiones
        (camiones + i * sizeof(camiones))->tiempoLlegada = rand() % (int)(configuracion->duracionJornadaMinutos + 1);
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
        fprintf(archPuerto, "B%03d;T=%d;C=%d01", b, *(tiemposBuques + i), b);

        // Loop for para registrar los contenedores de cada buque
        // Las condiciones revisan que no haya mas contenedores por buque que los aclarados en config.txt y que no haya mas contenedores que los establecidos en la variable de antes
        // Aca si se suma la variable de contenedores
        for(c = 1; c <= configuracion->maximoContenedoresPorBuque && contenedoresTotales < cantContenedores; c++, contenedoresTotales++)
        {
            // snprintf escribe a la variable del primero parametro (nroCont), la cantidad de bytes del segundo parametro (TAM_NOM_CONTENEDOR), lo que hay en el tercer parametro con formato como un printf normal
            snprintf(nroCont, TAM_NOM_CONTENEDOR, "C%d%02d", b, c);
            fprintf(archPuerto, ",C%d%02d", b, c);
            // Agrego los nombres de los contenedores a una cola para luego desacolar con los camiones
            if(ponerEnCola(contenedores, nroCont, strlen(nroCont)) != TODO_OK)
            {
                printf("Error al asignar memoria para cola de contenedores para puerto.txt\n");
                return ERR_MEMORIA;
            }
        }
    }

    // Saco los contenedores de la cola y los guardo en el campo de contenedor de cada camion del vector de camiones
    // (cuando este andando, fijarse si se puede saltear este paso y ponerlo directo donde se agrega a la cola, ahorrando toda la cola en si)
    i = 0;
    while(!colaVacia(contenedores))
    {
        if(sacarDeCola(contenedores, (camiones + i * sizeof(tCamion))->codigoContenedor, TAM_NOM_CONTENEDOR) != TODO_OK)
        {
            printf("Error al asignar memoria para sacar de cola\n");
            return ERR_MEMORIA;
        }
        i++;
    }

    // Ordenar el vector de camiones por tiempo de llegada
    qsort(camiones, configuracion->maximoCamiones, sizeof(int), cmpTiempo);

    // Registrar los camiones
    fprintf(archPuerto, "\n");
    fprintf(archPuerto, "\n");
    fprintf(archPuerto, "[CAMIONES]\n");
    for(k = 1; k <= configuracion->maximoCamiones; k++)
    {
        fprintf(archPuerto, "K%03d;T=%d;C=%s\n", k, (camiones + i * sizeof(tCamion))->tiempoLlegada, (camiones + i * sizeof(tCamion))->codigoContenedor);
    }

    free(tiemposBuques);
    free(camiones);
    crearBuquesyCamiones(puerto, archPuerto);
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
/* -------------------- hecho en la rama estructura_main previo a la fusion (termina aca) -------------------- */

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
    if(muelle->buqueAct == NULL ) // Verifica si hay ALGO en el muelle q pasa el operario
    {
        return MUELLE_VACIO;
    }

    // if(colaVacia(&((*muelle).buqueAct->cont_pend) ))
    if((&((*muelle).buqueAct->cont_pend) )) // verifica si hay contenedores pendientes en el buque de ese muelle
        return BUQUE_VACIO;                          // implementado con pila

    if(zonaDest->cantActual >= zonaDest->capacMax)  // verifica que haya espacio en la zona para ingresar contenedor
    {
        return ZONA_SIN_CAPACIDAD;
    }


    //desacolar( &((*muelle).buqueAct->cont_pend), contenedor_descarga, sizeof(tContenedor));
    sacarDePila(&(*muelle).buqueAct->cont_pend, contenedor_descarga, sizeof(tContenedor)); // implementado con pila
    apilar( &zonaDest->contenedores, contenedor_descarga, sizeof(tContenedor));

    // Contador de zona
    zonaDest->cantActual++;

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

int ejecutarOperacion(char *entradaUsuario, tPuerto *puerto, int tiempoAct, unsigned int *tiempoJornada, int puntuacion, tOperador *operador)
{
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// para borrar    v
    /*
    // entrada: "DES M1 Z1". puerto: estado actual de la simulacion. Basicamente entrada es la operacion y puerto es el estado.

    tOperacion operacion;
    int resultado;

    // parsear ->strtok
    // Un comando válido tiene como máximo 3 palabras (ej: DES M1 Z1).
    char *pal[MAX_CANT_PAL];
    int cantPal = 0;

    // strtok busca la primera palabra
    char *palabra = strtok(entradaUsuario, " \n");

    // Mientras encuentre una palabra
    while (palabra != NULL && cantPal < MAX_CANT_PAL)
    {
        pal[cantPal] = palabra;
        cantPal++;
        palabra = strtok(NULL, " \n"); // sig palabra de misma cadena
    }

    // caso error
    if (cantPal == 0)
    {
        return COMANDO_INVALIDO;
    }
    */
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// para borrar    ^
    tOperacion operacion;
    char aux[MAX_CODIGO];
    strcpy(aux, entradaUsuario);
    char *comando = strtok(aux, " \n\0");
    tMuelle *muelleDestino = NULL;
    tZona *zonaDestino = NULL;
    tZona *zonaOrigen = NULL;

    int resultado = detectarOperacion(comando, zonaOrigen, zonaDestino, muelleDestino, &operacion, puerto, tiempoAct, puntuacion);
    if(resultado != TODO_OK)
        return resultado;

    resultado = realizarEventos(puerto, &operacion, &tiempoAct, tiempoJornada);
    if(resultado != TODO_OK)
        return resultado;

    resultado = realizarOperacion(&operacion, zonaOrigen, zonaDestino, muelleDestino, puerto, tiempoAct, puntuacion);
    if(resultado != TODO_OK)
        return resultado;

    operador->puntajeTotal += operacion.puntuacion; // realiza suma de puntuación por cada operación finalizada
    operacion.tiempoDeFinalizacion = tiempoAct; // registro tiempo de finalizacion de operacion

    return OP_EXITO;
}

tMuelle* buscarMuelle(tPuerto *puerto, const char *idBuscado)
{
    // para buscar si existe el muelle
    int i=0;
    for (i = 0; i < puerto->cantMuelles; i++)
    {

        if (strcmp(puerto->muelles[i].codMuelle, idBuscado) == 0)
        {
            return &(puerto->muelles[i]); // Retorna dirección de memoria del muelle
        }
    }
    return NULL; // no encuentra, no esya
}

tZona* buscarZona(tPuerto *puerto, const char *idBuscado)
{

    tNodo *nodoActual = puerto->zonas;

    while (nodoActual != NULL)
    {

        tZona *zonaActual = (tZona *)nodoActual->info;

        // cod-ID
        if (strcmp(zonaActual->codZona, idBuscado) == 0)
        {
            return zonaActual; // zona existe
        }

        // Avanza
        nodoActual = nodoActual->sig;
    }

    return NULL; // no se encontró
}

int ejecutarREU( tZona* zonaOrigen, tZona* zonaDest, tContenedor* contenedor_desc)
{

    if( zonaOrigen->contenedores == NULL)
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

    desapilar( &zonaOrigen->contenedores,contenedor_desc,sizeof(tContenedor)); // saco de zona origen
    apilar(&zonaDest->contenedores, contenedor_desc, sizeof(tContenedor)); // apilo en zona destino


    zonaOrigen->cantActual --;
    zonaDest->cantActual++;
    return TODO_OK;
}

int desapilar( tPila *p,  void *d, unsigned tamInfo)
{


    if( *p == NULL)
        return ERROR_PILA;
    tNodo *nodo_aux = *p; // tope
    //nodo_aux->tamInfo = tamInfo;

    memcpy(d,nodo_aux->info,MIN(tamInfo,nodo_aux->tamInfo));
    *p = nodo_aux->sig;
    free(nodo_aux->info);
    free(nodo_aux);
    return TODO_OK;
}

void ejecutarVER( tPuerto* puerto, int tiempoActual, int puntuacionProv)
{
    tContenedor *prox;

    printf("\n\t\tESTADO DEL PUERTO\n");
    printf("Tiempo: %d\n", tiempoActual);
    printf("Puntuacion actual: %d\n", puntuacionProv);


    printf("\n MUELLES\n"); // buques atracados en muelle
    int i;
    for(i = 0; i<= puerto->cantMuelles; i++)
    {
        // Recorro segun cant muelles
        tMuelle *m = ((puerto->muelles)+i);
        // tMuelle *m = &puerto->muelles[i];
        if( m->buqueAct == NULL )
        {
            printf(" Muelle %s: Libre", m->codMuelle);
        }
        else
        {
            if( m->buqueAct->cont_pend == NULL )
            {
                printf("%s: %s -> Sin contenedores pendientes.\n", m->codMuelle, m->buqueAct->codigoBuque);
                //printf(" Cod Muelle %s ->", puerto->muelles[i]->codMuelle);
                //printf(" Buque atracado:%s\n", puerto->muelles[i]->buqueAct->codigoBuque);
            }
            else
            {
                //tContenedor *prox = (tContenedor*)m->buqueAct->cont_pend.pri->info;
                memcpy(&prox,m->buqueAct->cont_pend->info, m->buqueAct->cont_pend->tamInfo);
                printf("%s: %s -> proximo contenedor %s.\n", m->codMuelle, m->buqueAct->codigoBuque, prox->id);
            }
        }
    }

    int contador = 0; // Cant buques esperando
    tNodo *aux = puerto->buquesEsperando.pri; // punt auxiliar para recorrer cola de buques
    while( aux != NULL)
    {
        contador++;
        aux = aux->sig;
    }
    printf("Cantidad de buques esperando muelles disponibles: %d\n", contador);

    // contenido zonas de almacenamiento
    printf("\n --- Zonas de almacenamiento ---\n");
    tNodo *aux_zonas = puerto->zonas;
    while( aux_zonas != NULL)
    {
        tZona *zona = (tZona*)aux_zonas->info;
        printf("%s: [", zona->codZona);

        tNodo *nodoCont = zona->contenedores; // pila de contenedores propios de ESA zona
        while( nodoCont != NULL)
        {
            tContenedor *c = (tContenedor*)nodoCont->info;
            printf("%s",c->id);
            if( nodoCont->sig != NULL)
            {
                printf("|");
            }
            nodoCont = nodoCont->sig;
        }
        printf("]\n");
        aux_zonas = aux_zonas->sig; // siguiente zona
    }


}

tZona* buscarZonaTope(tLista zonas, const char *codContBuscado)
{
    tNodo *aux = zonas;
    while(aux != NULL)
    {
        tZona *zonaActual= (tZona*)aux->info;

        if( zonaActual->contenedores != NULL)
        {
            tNodo *nodoCont = zonaActual->contenedores;
            tContenedor *c = (tContenedor*)nodoCont->info;

            if( strcmp(c->id,codContBuscado)==0 )
            {
                return zonaActual;
            }
        }

        aux = aux->sig;
    }
    return NULL;
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
        strncpy(bufferCodigo,ptrBuffer+1,TAM_LINEA_CODIGO_CONT);
        bufferCodigo[TAM_LINEA_CODIGO_CONT]='\0'; // buffer tendra vector de char que ira a pila de codigos de buque actual
        ponerEnPila(&buque->cont_pend, bufferCodigo, strlen(bufferCodigo)); /// PENDIENTE PARA CUANDO PASE TODO A PILA
        *ptrBuffer = '\0';
    }

    ptrBuffer = strrchr(buffer, ';');   // busco el ultimo C= con ";"
    strncpy(bufferCodigo,ptrBuffer+3,TAM_LINEA_CODIGO_CONT);; // ptrBuffer+3 salto ;C= y queda en C de C001
    bufferCodigo[TAM_LINEA_CODIGO_CONT]='\0';
    ponerEnPila(&buque->cont_pend, bufferCodigo, strlen(bufferCodigo)); // ultima carga al fondo de pila y primera queda en tope
    /// PENDIENTE PARA CUANDO PASE TODO A PILA
    *ptrBuffer = '\0';

    ptrBuffer = strrchr(buffer, ';');
    sscanf(ptrBuffer+3,"%u", &buque->horaLlegada);
    //buque->tiempoLlegada = atoi(ptrBuffer+3);
    *ptrBuffer = '\0';

    strncpy(buque->codigoBuque,buffer,TAM_NOMBRE);
    buque->codigoBuque[TAM_NOMBRE]='\0';
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
    strncpy(camion->codigoContenedor,ptrBuffer+3,TAM_LINEA_CODIGO_CONT); // ptrBuffer+3 salto ;C= y queda en C de C001
    *ptrBuffer = '\0';

    ptrBuffer = strrchr(buffer, ';');
    sscanf(ptrBuffer+3,"%u", &camion->tiempoLlegada);
    *ptrBuffer = '\0';

    strncpy(camion->codigoCamion,buffer,TAM_NOMBRE);
    camion->codigoCamion[TAM_NOMBRE]='\0';
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

    crearCola(&puerto->buquesArchivo);
    crearCola(&puerto->camionesArchivo);
    crearPila(&buque.cont_pend); /// creo pila dinamica para contenedores dentro de buque

    /// Inicio buscando buques
    // leo hasta llegar a [BUQUES]
    while(fgets(buffer, TAM_LINEA_BUFF_LECTURA, pfPuerto) && comparacion != 0)
    {
        comparacion = strncmp(buffer, "[BUQUES]", 8);
    }

    // caso: llego al final del archivo y no hallo CAMIONES
    if(comparacion != 0)
    {
        return NO_HALLADO;
    }

    while(fgets(buffer, TAM_LINEA_BUFF_LECTURA, pfPuerto))
    {
        if(parsearBuques(&buque, buffer, bufferCodigo)!=TODO_OK)
        {
            return NO_HALLADO;
        }
        ponerEnCola(&puerto->buquesArchivo, &buque,sizeof(tBuque));
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
    {
        return NO_HALLADO;
    }

    while(fgets(buffer, TAM_LINEA_BUFF_LECTURA, pfPuerto))
    {
        if(parsearCamiones(&camion, buffer)!=TODO_OK)
        {
            return NO_HALLADO;
        }
        ponerEnCola(&puerto->camionesArchivo, &camion, sizeof(tCamion));
    }

    return TODO_OK;
}

int realizarEventos(tPuerto *puerto, tOperacion *operacion,
                    int *temporizador, unsigned int *tiempoJornada)
{
    /// PARA realizarEventos
    unsigned tiempoLimite = 0;
    int resultado;

    tiempoLimite = *temporizador + operacion->tiempoPorDefault;
    if(tiempoLimite > *tiempoJornada) // detección de fin de jornada
        return FIN_JORNADA; // no alcanza el tiempo para ejecutar la operación

    while(*temporizador<tiempoLimite)
    {
        if((resultado = vigiladorBuques(puerto, temporizador, &operacion->tiempoPorDefault))!=TODO_OK)// llegada buques
            return resultado;

        if((resultado = vigiladorCamiones(puerto, temporizador))!=TODO_OK) // llegada camiones
            return resultado;

        (*temporizador)++;
    }

    // caso limite temp == tiempoLim
    if((resultado = vigiladorBuques(puerto, temporizador, &operacion->tiempoPorDefault))!=TODO_OK)// llegada buques
        return resultado;

    if((resultado = vigiladorCamiones(puerto, temporizador))!=TODO_OK) // llegada camiones
        return resultado;

    return TODO_OK;
}

int vigiladorBuques(tPuerto *puerto, const int *temporizador, const int *tiempoPorDefault)
{
    tBuque buque;

    if(colaVacia(&(puerto->buquesArchivo)))
        return FIN_BUQUE;

    sacarDeCola(&(puerto->buquesArchivo), &buque, sizeof(buque));

    if((buque.horaLlegada == *temporizador)) // evaluo si buque llego en temporizador actual
    {
        if(!asignarMuelle(&buque, puerto->muelles)) // mando a muelle si no puedo mando a cola de espera BIEN SERIA (buque, puerto.muelle)
            ponerEnCola(&puerto->buquesEsperando, &buque, sizeof(buque)); // cola de espera
    }

    return TODO_OK;
}

int vigiladorCamiones(tPuerto *puerto, const int *temporizador)
{
    tCamion camion;
    int resultado;

    if(colaVacia(&puerto->camionesArchivo))
        return FIN_CAMIONES;

    sacarDeCola(&puerto->camionesArchivo, &camion, sizeof(tCamion));

    if(camion.tiempoLlegada == *temporizador) // evaluo si camion llego en temporizador actual
    {
        // solo ponemos en cola para lo demas esperar ACCION de operador
        if((resultado = ponerEnCola(&puerto->camionesEsperando, &camion, sizeof(camion)))!=TODO_OK)
            return resultado;
    }

    return TODO_OK;
}

int asignarMuelle(tBuque *buque, tMuelle *muelle)
{
    return TODO_OK;
    // revisar que contiene estructura tMuelle y como esta seteado numero de muelles
}

int detectarOperacion(const char* comando, tZona *zonaOrigen, tZona* zonaDestino, tMuelle *muelleDestino, tOperacion *operacion,
                      tPuerto *puerto, int tiempoAct, int puntuacion)
{
    if (strcmp(comando, "DES") == 0)
    {
        //  (DES, Muelle, Zona)

        char *param1 = strtok(NULL, " "); // Muelle -> M1 // usa NULL porque la vez anterior que se uso fue con aux, es decir, con la copia de la entrada del usuario
        char *param2 = strtok(NULL, " \n\0"); // Zona -> Z1

        //
        muelleDestino = buscarMuelle(puerto, param1);
        if( muelleDestino == NULL)
            return MUELLE_INCORRECTO;

        zonaDestino = buscarZona(puerto, param2);
        if( zonaDestino == NULL)
            return ZONA_INCORRECTA; // osea zona incorrecta

        operacion->codOperacion = OPERACION_DES;
        return TODO_OK;
    }
    else if (strcmp(comando, "REU") == 0)
    {
        char *param1 = strtok(NULL, " ");
        char *param2 = strtok(NULL, " \n\0");

        // ejecutarREU
        zonaOrigen = buscarZona(puerto, param1);
        if( zonaOrigen == NULL)
        {
            return ZONA_INCORRECTA;
        } // osea zona incorrecta

        zonaDestino = buscarZona(puerto, param2);
        if( zonaDestino == NULL)
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
                      tPuerto *puerto, int tiempoAct, int puntuacion)
{
    switch (operacion->codOperacion)
    {
    case OPERACION_DES:
    {
        tContenedor contenedorDescargado; // la aux
        int resultado = ejecutarDES(muelleDestino, zonaDestino, &contenedorDescargado);
        if( resultado != OP_EXITO)
        {
            printf("Codigo de error: %d.", resultado);
            return resultado;
        }
        printf("Contenedor %s colocado en zona %s\n", contenedorDescargado.id,zonaDestino->codZona);
        return OP_EXITO;
    }


    break;

    case OPERACION_REU:
    {
        tContenedor contenedor_reubicar;
        int resultado = ejecutarREU(zonaOrigen,zonaDestino, &contenedor_reubicar);
        if( resultado != TODO_OK)
        {
            printf("Error: %d\n", resultado);
            return resultado;
        }
        printf(" El contenedor %s ha sido reubicado en la zona %s desde %s.\n", contenedor_reubicar.id, zonaDestino->codZona, zonaOrigen->codZona);

    }

    break;

    case OPERACION_VER:
    {
        ejecutarVER(puerto, tiempoAct, puntuacion);
    }

    break;

    case OPERACION_ENT:
    {
        tContenedor contenedor;                                 /// REVISAR -> es un placeholder
        tCamion camion;                                         /// REVISAR -> es un placeholder
        ejecutarENT(puerto, &contenedor, &camion);              /// REVISAR -> lo que se manda no va a tener sentido, revisar que mandar para que sirva y como procesar lo que hace y devuelve
    }

    break;

    case OPERACION_ESP:
    {
        ejecutarESP(puerto, tiempoAct, 2); /// OJO hardcodeado luego pasar a variable de configu
    }

    break;
    default:
    {
        printf("Operacion no hallada");
    }
    }
    return TODO_OK;
}

void ejecutarESP()
{

}
/*
int ejecutarENT( tPuerto* puerto)
{
    return TODO_OK;
}
*/

int ejecutarENT( tPuerto* puerto, tContenedor* contenedorDesc, tCamion *camionAtendido)
{
    if( puerto->camionesEsperando.pri  == NULL ) // verifico existencia de camiones
        return NO_HAY_CAMIONES;

    tCamion *camionFrente = (tCamion*)puerto->camionesEsperando.pri->info;// contenedor solicitado por camion atendido

    tZona *zonaContenedor = buscarZonaTope(puerto->zonas, camionFrente->codigoContenedor); // busco zona donde este contenedor ej C101
    if( zonaContenedor == NULL)
    {
        return CONTENEDOR_NO_DISPONIBLE;
    }

    if( desapilar(&zonaContenedor->contenedores, contenedorDesc, sizeof(tContenedor)) !=  TODO_OK )
    {
        return ERROR_PILA;
    }
    zonaContenedor->cantActual--; // le saco un contenedor
    if( desacolar(&puerto->camionesEsperando, camionAtendido, sizeof(tCamion)) != TODO_OK )
    {
        return ERROR_PILA;
    }


    return TODO_OK;
}
