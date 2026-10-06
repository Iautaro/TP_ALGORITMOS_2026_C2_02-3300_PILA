#include "funciones.h"

int realizarEventos(tCola *buques, tCola *camiones, tMuelle *muelle, tOperacion *operacion, unsigned *temporizador, FILE *pfOperador, tOperador *operador,
                    char *nombreArchivoPuerto, unsigned posUltimoBuque, unsigned posUltimoCamion, unsigned *tiempoJornada)
{

    /// posUltimoBuque y  posUltimoCamion
    // guardo pos de ultimo buque/camion leido en archivo, variable que va en main.

    unsigned tiempoLimite = 0;
    FILE *pfPuerto = fopen(nombreArchivoPuerto,"rt");
    if(!pfPuerto)
    {
        printf("Error al abrir el archivo %s", nombreArchivoPuerto);
        return ERR_ARCHIVO;
    }

    tiempoLimite = *temporizador + operacion->tiempoPorDefault;
    if(tiempoLimite > *tiempoJornada) // detección de fin de jornada
    {
        return FIN_JORNADA;
    }

    while(*temporizador<tiempoLimite)
    {
        if(vigiladorBuques(buques, muelle, temporizador, &operacion->tiempoPorDefault, pfPuerto, posUltimoBuque)!=TODO_OK)// llegada buques
        {
            fclose(pfPuerto);
            return ERR_VIGILADOR;
        }
        if(vigiladorCamiones(camiones, temporizador, pfPuerto, posUltimoCamion)!=TODO_OK) // llegada camiones
        {
            fclose(pfPuerto);
            return ERR_VIGILADOR;
        }
        (*temporizador)++;
    }

    printf("Finalizada la operación %s", operacion->nombre); // informar/realiza operaciones finalizadas
    operador->puntajeTotal += operacion->puntuacion; // realiza suma de puntuación por cada operación finalizada
    operacion->tiempoDeFinalizacion = *temporizador; // registro tiempo de finalizacion de operacion
    fclose(pfPuerto);

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

int vigiladorBuques(tCola *buques, tMuelle *muelle, const unsigned *temporizador, const int *tiempoPorDefault, FILE *pfPuerto, unsigned posUltimoBuque)
{
    char hallado = 0;
    tBuque buque;
    crearPila(&buque.codigoContenedores); /// creo pila dinamica para contenedores dentro de buque

    char buffer[TAM_LINEA_BUFF_LECTURA];
    char bufferCodigo[TAM_LINEA_CODIGO_CONT];

    // primera pasada debo leer hasta llegar a primer buque
    if(posUltimoBuque == 0)
    {
        while(fgets(buffer, TAM_LINEA_BUFF_LECTURA, pfPuerto) && !hallado)
        {
            if(strncmp(buffer, "[BUQUES]", 8) == 0)
            {
                hallado = 1;
            }
        }
        if(!hallado) // no encontramos BUQUE
        {
            return NO_HALLADO;
        }

    }
    else
    {
        fseek(pfPuerto, posUltimoBuque, SEEK_SET); // uso posUltimoBuque para saltar los ya leidos
    }

    fgets(buffer, TAM_LINEA_BUFF_LECTURA, pfPuerto);

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
        ponerEnPila(&buque.codigoContenedores, bufferCodigo, strlen(bufferCodigo));
        *ptrBuffer = '\0';
    }

    ptrBuffer = strrchr(buffer, ';');   // busco el ultimo C= con ";"
    strncpy(bufferCodigo,ptrBuffer+3,TAM_LINEA_CODIGO_CONT);; // ptrBuffer+3 salto ;C= y queda en C de C001
    bufferCodigo[TAM_LINEA_CODIGO_CONT]='\0';
    ponerEnPila(&buque.codigoContenedores, bufferCodigo, strlen(bufferCodigo)); // ultima carga al fondo de pila y primera queda en tope
    *ptrBuffer = '\0';

    ptrBuffer = strrchr(buffer, ';');
    sscanf(ptrBuffer+3,"%u", &buque.tiempoLlegada);
    //buque.tiempoLlegada = atoi(ptrBuffer+3);
    *ptrBuffer = '\0';

    strncpy(buque.codigoBuque,buffer,TAM_NOMBRE);
    buque.codigoBuque[TAM_NOMBRE]='\0';
    // fin desarmar buffer

    if((buque.tiempoLlegada == *temporizador)) // evaluo si buque llego en temporizador actual
    {
        if(!asignarMuelle(&buque, muelle)) // mando a muelle si no puedo mando a cola de espera BIEN SERIA (buque, puerto.muelle)
        {
            ponerEnCola(buques, &buque, sizeof(buque)); // cola de espera
        }
        posUltimoBuque = ftell(pfPuerto); // guardo posicion de ultimo leido en archivo para proxima vuelta
        // si no se leyo nada no se guarda ultima pos (evito saltearlo)
    }

    rewind(pfPuerto);
    return TODO_OK;
}

int vigiladorCamiones(tCola *camiones, const unsigned *temporizador, FILE *pfPuerto, unsigned posUltimoCamion)
{
    char hallado = 0;
    tCamion camion;
    char buffer[TAM_LINEA_BUFF_LECTURA];

    // primera pasada debo leer hasta llegar a primer camion
    if(posUltimoCamion == 0)
    {
        while(fgets(buffer, TAM_LINEA_BUFF_LECTURA, pfPuerto) && !hallado)
        {
            if(strncmp(buffer, "[CAMIONES]", 10) == 0)
            {
                hallado = 1;
            }
        }
        if(!hallado) // no encontramos CAMION
        {
            return NO_HALLADO;
        }
    }
    else
    {
        fseek(pfPuerto, posUltimoCamion, SEEK_SET); // uso posUltimoCamion para saltar los ya leidos
    }

    fgets(buffer, TAM_LINEA_BUFF_LECTURA, pfPuerto);

    // salgo si no encontro un camion donde debería estar
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
    strncpy(camion.codigoContenedor,ptrBuffer+3,TAM_LINEA_CODIGO_CONT); // ptrBuffer+3 salto ;C= y queda en C de C001
    *ptrBuffer = '\0';

    ptrBuffer = strrchr(buffer, ';');
    sscanf(ptrBuffer+3,"%u", &camion.tiempoLlegada);
    *ptrBuffer = '\0';

    strncpy(camion.codigoCamion,buffer,TAM_NOMBRE);
    camion.codigoCamion[TAM_NOMBRE]='\0';
    // fin desarmar buffer

    if(camion.tiempoLlegada == *temporizador) // evaluo si camion llego en temporizador actual
    {
        // solo ponemos en cola para lo demas esperar ACCION de operador
        ponerEnCola(camiones, &camion, sizeof(camion));
        posUltimoCamion = ftell(pfPuerto); // guardo posicion de ultimo leido en archivo para proxima vuelta
        // si no se lleyo nada no se guarda ultima pos (evito saltearlo)
    }

    rewind(pfPuerto);
    return TODO_OK;
}

int asignarMuelle(tBuque *buque, tMuelle *muelle)
{
    return TODO_OK;
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
