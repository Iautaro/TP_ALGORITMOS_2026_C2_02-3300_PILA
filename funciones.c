#include "funciones.h"


int colaVacia(const tCola *c)
{
    return c->pri == NULL;
}

int desacolar( tCola *c, void *info, unsigned tam)
{
    if( c->pri == NULL)
        return 1; // error

    tNodo *elim = c->pri;
    c->pri = elim->sig;

    if( c->pri == NULL){
        c->ult = NULL;}

    memcpy(info,elim->info,MIN(tam,elim->tamInfo) );
    free( elim->info);
    free(elim);

    return TODO_OK;
}

int apilar( tPila *p, const void *d, unsigned tamInfo)
{
    tNodo *nuevoNodo = malloc( sizeof(tNodo));
    if( nuevoNodo == NULL)
        return 1; // fallo
    nuevoNodo->info = malloc(tamInfo);
    if( nuevoNodo->info == NULL)
    {
        free(nuevoNodo);
        return 1; // fallo
    }
    memcpy( nuevoNodo->info, d,tamInfo);
    nuevoNodo->tamInfo = tamInfo;

    nuevoNodo->sig = *p;
    *p = nuevoNodo;
    return TODO_OK;

}

// ---------------------------------------------------------------

int ejecutarDES( tMuelle *muelle, tZona *zonaDest, tContenedor *contenedor_descarga) // logica de descargar y almacenar
{
    if(muelle->buqueAct == NULL ) // Verifica si hay ALGO en el muelle q pasa el operario
    {
        return MUELLE_VACIO;
    }

    if( colaVacia(&((*muelle).buqueAct->cont_pend) )) // verifica si hay contenedores pendientes en el buque de ese muelle
        return BUQUE_VACIO;

    //if(pilaLlena(zonaDest, sizeof(tContenedor))) // verifica que haya espacio en la zona para ingresar contenedor
    if( zonaDest->cantActual >= zonaDest->capacMax){
        return ZONA_SIN_CAPACIDAD;}

    //desacolar( &muelle->buqueAct->cont_pend, contenedor_descarga, sizeof(tContenedor));
    desacolar( &((*muelle).buqueAct->cont_pend), contenedor_descarga, sizeof(tContenedor));
    apilar( &zonaDest->contenedores, contenedor_descarga, sizeof(tContenedor));

    // COntador de zona
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
    if( leidos > cant_esperada){
        printf("\nCantidad de parametros mayor a la esperada(%d).", cant_esperada);
        return EXCESO_PARAMETROS;
    }
    return TODO_OK;
}



int ejecutarOperacion(char *entradaUsuario, tPuerto *puerto, int tiempoAct, int puntuacion)
{ // entrada: "DES M1 >1". puerto: estado actual de la simulacion. Basicamente entrada es la operacion y puerto es el estado.

    int i=0;
    for(i = 0; entradaUsuario[i] != '\0'; i++) {
        entradaUsuario[i] = toupper(entradaUsuario[i]);// convierto a mayusc
    }

    // parsear ->strtok
    // Un comando válido tiene como máximo 3 palabras (ej: DES M1 Z1).
    char *pal[MAX_CANT_PAL];
    int cantPal = 0;

    // strtok busca la primera palabra
    char *palabra = strtok(entradaUsuario, " \n");

    // Mientras encuentre una palabra
    while (palabra != NULL && cantPal < MAX_CANT_PAL) {
        pal[cantPal] = palabra;
        cantPal++;
        palabra = strtok(NULL, " \n"); // dig palabra de misma cadena
    }

    // caso error
    if (cantPal == 0) {
        return COMANDO_INVALIDO;
    }


    char *comando = pal[0]; // "DES"

    // if - else- switch ??
    if (strcmp(comando, "DES") == 0) {
        //  (DES, Muelle, Zona)
        int validacion = validarCantidad(cantPal, 3);
        if (validacion != OP_EXITO){ return validacion;}

        char *param1 = pal[1]; // Muelle -> M1
        char *param2 = pal[2]; // Zona -> Z1


        if (param1[0] != 'M' || param2[0] != 'Z') {
            printf("Error: Muelle debe empezar con 'M' y Zona con 'Z'.\n"); // temporal
            return PARAMETRO_INCORRECTO;
        }

        //
        tMuelle *muelleDestino = buscarMuelle(puerto, param1);
        if( muelleDestino == NULL)
            return MUELLE_INCORRECTO;

        tZona *zonaDestino = buscarZona(puerto, param2);
        if( zonaDestino == NULL)
            return ZONA_INCORRECTA; // osea zona incorrecta

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
    else if (strcmp(comando, "REU") == 0) {
        int validacion = validarCantidad(cantPal, 3);
        if (validacion != OP_EXITO) {return validacion;}

        char *param1 = pal[1];
        char *param2 = pal[2];

        if (param1[0] != 'Z' || param2[0] != 'Z') {
            printf("Error: Ambas zonas deben empezar con 'Z'.\n");
            return PARAMETRO_INCORRECTO;
        }

        // ejecutarREU
        tZona *zonaOrigen = buscarZona(puerto, param1);
        if( zonaOrigen == NULL){
            return ZONA_INCORRECTA;} // osea zona incorrecta

        tZona *zonaDestino = buscarZona(puerto, param2);
        if( zonaDestino == NULL){
            return ZONA_INCORRECTA;}// osea zona incorrecta

        tContenedor contenedor_reubicar;
        int resultado = ejecutarREU(zonaOrigen,zonaDestino, &contenedor_reubicar);
        if( resultado != TODO_OK){
            printf("Error: %d\n", resultado);
            return resultado;
        }

        printf(" El contenedor %s ha sido reubicado en la zona %s desde %s.\n", contenedor_reubicar.id, zonaDestino->codZona, zonaOrigen->codZona);
        return TODO_OK;
    }
    else if (strcmp(comando, "ENT") == 0  || strcmp(comando, "VER") == 0 ||strcmp(comando, "ESP") == 0) { // provisorio, puedo separar en 3 dist
        //
        int validacion = validarCantidad(cantPal, 1);
        if (validacion != OP_EXITO)
        {return validacion;}

        // ejecutarENT, ejecutarVER, ejecutarESP
        if(strcmp(comando, "VER") == 0){
                ejecutarVER(puerto, tiempoAct, puntuacion);
                return TODO_OK;
        }
    }
    else {
        // El comando ingresado no existe
        printf("Error: Comando '%s' no reconocido.\n", comando);
        return COMANDO_INVALIDO;
    }

    return OP_EXITO;
}



tMuelle* buscarMuelle(tPuerto *puerto, const char *idBuscado)
{ // para buscar si existe el muelle
    int i=0;
    for (i = 0; i < puerto->cantMuelles; i++) {

        if (strcmp(puerto->muelles[i].codMuelle, idBuscado) == 0) {
            return &(puerto->muelles[i]); // Retorna dirección de memoria del muelle
        }
    }
    return NULL; // no encuentra, no esya
}



tZona* buscarZona(tPuerto *puerto, const char *idBuscado)
{

    tNodo *nodoActual = puerto->zonas;

    while (nodoActual != NULL) {

        tZona *zonaActual = (tZona *)nodoActual->info;

        // cod-ID
        if (strcmp(zonaActual->codZona, idBuscado) == 0) {
            return zonaActual; // zona existe
        }

        // Avanza
        nodoActual = nodoActual->sig;
    }

    return NULL; // no se encontró
}

int ejecutarREU( tZona* zonaOrigen, tZona* zonaDest, tContenedor* contenedor_desc){

    if( zonaOrigen->contenedores == NULL){
        return ZONA_ORIGEN_VACIA;}

    if( strcmp(zonaOrigen->codZona, zonaDest->codZona) == 0 ){
        return ZONAS_IGUALES;}

    if( zonaDest->cantActual >= zonaDest->capacMax){
        return ZONA_SIN_CAPACIDAD;}

    desapilar( &zonaOrigen->contenedores,contenedor_desc,sizeof(tContenedor)); // saco de zona origen
    apilar(&zonaDest->contenedores, contenedor_desc, sizeof(tContenedor)); // apilo en zona destino


    zonaOrigen->cantActual --;
    zonaDest->cantActual++;
    return TODO_OK;
}

int desapilar( tPila *p,  void *d, unsigned tamInfo){


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
        {   if( m->buqueAct->cont_pend.pri == NULL )
            {
            printf("%s: %s -> Sin contenedores pendientes.\n", m->codMuelle, m->buqueAct->codigoBuque);
            //printf(" Cod Muelle %s ->", puerto->muelles[i]->codMuelle);
            //printf(" Buque atracado:%s\n", puerto->muelles[i]->buqueAct->codigoBuque);
            } else{
            tContenedor *prox = (tContenedor*)m->buqueAct->cont_pend.pri->info;
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

        if( zonaActual->contenedores != NULL){
            tNodo *nodoCont = zonaActual->contenedores;
            tContenedor *c = (tContenedor*)nodoCont->info;

            if( strcmp(c->id,codContBuscado)==0 ){
                return zonaActual;
            }
        }

        aux = aux->sig;
    }
    return NULL;
}

/// Parte Nic

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
        ponerEnPila(&buque->cont_pend, bufferCodigo, strlen(bufferCodigo));
        *ptrBuffer = '\0';
    }

    ptrBuffer = strrchr(buffer, ';');   // busco el ultimo C= con ";"
    strncpy(bufferCodigo,ptrBuffer+3,TAM_LINEA_CODIGO_CONT);; // ptrBuffer+3 salto ;C= y queda en C de C001
    bufferCodigo[TAM_LINEA_CODIGO_CONT]='\0';
    ponerEnPila(&buque->cont_pend, bufferCodigo, strlen(bufferCodigo)); // ultima carga al fondo de pila y primera queda en tope
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

int crearBuquesyCamiones(tCola *buquesArchivo, tCola *camionesArchivo, FILE *pfPuerto)
{
    tBuque buque;
    tCamion camion;
    int comparacion = 1;

    char buffer[TAM_LINEA_BUFF_LECTURA];
    char bufferCodigo[TAM_LINEA_CODIGO_CONT];

    crearCola(buquesArchivo);
    crearCola(camionesArchivo);
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
        ponerEnCola(buquesArchivo , &buque,sizeof(tBuque));
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
        ponerEnCola(camionesArchivo, &camion, sizeof(tCamion));
    }

    return TODO_OK;
}

/**
-- CODIGO PROVISIONAL SOLO PARA TEST --
*/

void crearCola(tCola *p)
{
    p->pri = NULL;
    p->ult = NULL;
}

void crearPila(tPila *p)
{
    *p = NULL;
}

int ponerEnCola(tCola *p, const void *d, unsigned cantBytes)
{
    tNodo *nue = (tNodo *) malloc(sizeof(tNodo));

    if(nue == NULL || (nue->info = malloc(cantBytes)) == NULL)
    {
        free(nue);
        return 0;
    }

    memcpy(nue->info, d, cantBytes);
    nue->tamInfo = cantBytes;
    nue->sig = NULL;

    if(p->ult)
        p->ult->sig = nue;
    else
        p->pri = nue;

    p->ult = nue;

    return 1;
}

int ponerEnPila(tPila *p, const void *d, unsigned cantBytes)
{
    tNodo *nue;

    if((nue = (tNodo *)malloc(sizeof(tNodo))) == NULL ||
            (nue->info = malloc(cantBytes)) == NULL)
    {
        free(nue);
        return 0;
    }

    memcpy(nue->info, d, cantBytes);
    nue->tamInfo = cantBytes;
    nue->sig = *p;
    *p = nue;

    return 1;
}

