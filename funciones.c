#include "funciones.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define TODO_OK 0
#define MUELLE_VACIO 1
#define BUQUE_VACIO 2
#define ZONA_SIN_CAPACIDAD 3
#define ERROR_PILA 4
#define ZONA_INCORRECTA -3
#define MUELLE_INCORRECTO -4

#define OP_EXITO 0
#define COMANDO_INVALIDO -1
#define FALTAN_PARAMETROS -2
#define PARAMETRO_INCORRECTO -3
#define EXCESO_PARAMETROS -4

#define ZONA_ORIGEN_VACIA -5
#define ZONAS_IGUALES -6

//#define MAX_CANT_PAL 10
#define MAX_CANT_PAL 4
#define MIN(a,b) ((a) < (b)? (a):(b))


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
/*
int pilaLlena( const tPila *p, unsigned tam)
{
     tNodo *aux = malloc( sizeof(tNodo));
     if( aux == NULL)
        return 1;

     //void *info_aux = malloc( tam);
     aux->info = malloc( sizeof(tam));
     if( aux->info == NULL)
     {
         free(aux);
         return ERROR_PILA;
     }

}

int pilaLlena( const tPila *p, unsigned tam)
{
    tNodo *aux = malloc(sizeof(tNodo));
    void *aux_info = malloc(tam);

    if(aux == NULL || aux_info == NULL)
    {
        free(aux);
        free(aux_info);
        return ERROR_PILA; // memoria llena
    }
        free(aux);
        free(aux_info);
        return TODO_OK;// disponible espacio
}

*/
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
    int i=0;
    for(i; i<= puerto->cantMuelles; i++){ // Recorro segun cant muelles
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
