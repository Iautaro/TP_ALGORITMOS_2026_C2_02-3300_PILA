#include "funciones.h"

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
        ponerEnPila(&buque->codigoContenedores, bufferCodigo, strlen(bufferCodigo));
        *ptrBuffer = '\0';
    }

    ptrBuffer = strrchr(buffer, ';');   // busco el ultimo C= con ";"
    strncpy(bufferCodigo,ptrBuffer+3,TAM_LINEA_CODIGO_CONT);; // ptrBuffer+3 salto ;C= y queda en C de C001
    bufferCodigo[TAM_LINEA_CODIGO_CONT]='\0';
    ponerEnPila(&buque->codigoContenedores, bufferCodigo, strlen(bufferCodigo)); // ultima carga al fondo de pila y primera queda en tope
    *ptrBuffer = '\0';

    ptrBuffer = strrchr(buffer, ';');
    sscanf(ptrBuffer+3,"%u", &buque->tiempoLlegada);
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
    crearPila(&buque.codigoContenedores); /// creo pila dinamica para contenedores dentro de buque

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

int realizarEventos(tCola *buquesEsperando, tCola *camionesEsperando, tCola *buquesArchivo, tCola *camionesArchivo, tMuelle *muelle, tOperacion *operacion,
                    tOperador *operador, unsigned *temporizador, unsigned *tiempoJornada)
{

    unsigned tiempoLimite = 0;

    tiempoLimite = *temporizador + operacion->tiempoPorDefault;
    if(tiempoLimite > *tiempoJornada) // detección de fin de jornada
    {
        return FIN_JORNADA;
    }

    while(*temporizador<tiempoLimite)
    {
        if(vigiladorBuques(buquesEsperando, buquesArchivo, muelle, temporizador, &operacion->tiempoPorDefault)!=TODO_OK)// llegada buques
        {
            return ERR_VIGILADOR;
        }
        if(vigiladorCamiones( camionesEsperando, camionesArchivo, temporizador)!=TODO_OK) // llegada camiones
        {
            return ERR_VIGILADOR;
        }
        (*temporizador)++;
    }

    printf("Finalizada la operación %s", operacion->nombre); // informar/realiza operaciones finalizadas
    operador->puntajeTotal += operacion->puntuacion; // realiza suma de puntuación por cada operación finalizada
    operacion->tiempoDeFinalizacion = *temporizador; // registro tiempo de finalizacion de operacion

    /**
    --Observaciones [INICIO]--

    Esto es de Mati por ahora posponer y luego revisar si vector
    no es mas viable, guardar en variable operador y luego pasar a vector
    para hacer archivo nuevo ya ordenado

    char buffer[TAM_LINEA_BUFF_LECTURA];
    sprintf(buffer, "Operador: %s - Ranking: %d\n", operador->nombre, operador->puntajeTotal)
    int cant = strlen(buffer);
    /// VALIDAR SI buffer con "\n" se ejecuta el salto
    /// validar que escriba \0 en buffer luego de hacer sprintf
    fprintf(pfOperador, "%s", buffer); /// OJO VARIACION DE %d si lo cuenta igual 1 o 100
    fseek(pfOperador, -cant,SEEK_SET); // retrocedo por si debo actualizar de nuevo en otra iteracion
    // esto es necesario? se podria dejar al final de ejecucion

    /// NUEVA OPERACION A HACER
    // al finalizar toda la jornada y antes de volver a mostrar pantalla llamar a reorganizar puntaje para acomodar al nuevo operador en el archivo de ranking
    // usar variable nuevoOperador para decidir si ejecutar algoritmo de reorganizacion de ranking.

    -- Observaciones [FIN] --
    */

    return TODO_OK;
}

int vigiladorBuques(tCola *buquesEsperando, tCola *buquesArchivo, tMuelle *muelle,
                     const unsigned *temporizador, const int *tiempoPorDefault)
{
    tBuque buque;

    if(colaVacia(buquesArchivo))
    {
        return FIN_BUQUE;
    }

    sacarDeCola(buquesArchivo, &buque, sizeof(buque));

    if((buque.tiempoLlegada == *temporizador)) // evaluo si buque llego en temporizador actual
    {
        if(!asignarMuelle(&buque, muelle)) // mando a muelle si no puedo mando a cola de espera BIEN SERIA (buque, puerto.muelle)
        {
            ponerEnCola(buquesEsperando, &buque, sizeof(buque)); // cola de espera
        }
    }

    return TODO_OK;
}

int vigiladorCamiones(tCola *camionesEsperando, tCola *camionesArchivo, const unsigned *temporizador)
{
    tCamion camion;

    if(colaVacia(camionesArchivo))
    {
        return FIN_CAMIONES;
    }

    sacarDeCola(camionesArchivo, &camion, sizeof(tCamion));

    if(camion.tiempoLlegada == *temporizador) // evaluo si camion llego en temporizador actual
    {
        // solo ponemos en cola para lo demas esperar ACCION de operador
        ponerEnCola(camionesEsperando, &camion, sizeof(camion));
    }

    return TODO_OK;
}

int asignarMuelle(tBuque *buque, tMuelle *muelle)
{
    return TODO_OK;
    // revisar que contiene estructura tMuelle y como esta seteado numero de muelles
}

/**
-- CODIGO PROVISIONAL SOLO PARA TEST --
*/

void crearPila(tPila *p)
{
    *p = NULL;
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

void crearCola(tCola *p)
{
    p->pri = NULL;
    p->ult = NULL;
}

int sacarDeCola(tCola *p, void *d, unsigned cantBytes)
{
    tNodo *aux = p->pri;
    if(aux == NULL)
    return 0;
    p->pri = aux->sig;
    memcpy(d, aux->info, minimo(aux->tamInfo, cantBytes));
    free(aux->info);
    free(aux);
    if(p->pri == NULL)
    p->ult = NULL;
    return 1;
}

int colaVacia(const tCola *p)
{
    return p->pri == NULL;
}
