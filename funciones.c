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



int ejecutarOperacion(char *entradaUsuario, tPuerto *puerto)
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
    }
    else if (strcmp(comando, "ENT") == 0 || strcmp(comando, "VER") == 0 || strcmp(comando, "ESP") == 0) { // provisorio, puedo separar en 3 dist
        //
        int validacion = validarCantidad(cantPal, 1);
        if (validacion != OP_EXITO)
        {return validacion;}

        // ejecutarENT, ejecutarVER, ejecutarESP
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
