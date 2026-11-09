#include <stdio.h>
#include <string.h>
#include "operador.h"
#include "indice_operador.h"

int compararClaveIndice(const void *a, const void *b) // comparamos por Nombre y Puntaje
{
    const tIndice *idxA = (const tIndice*)a;
    const tIndice *idxB = (const tIndice*)b;

    return strcmp(idxA->nombreOperador, idxB->nombreOperador);
}

void mostrarIndice(const tIndice *pi, unsigned nivel)
{
    while(nivel--)
        printf("|");

    printf("- indice:%3u - %s\n",
           pi->indiceRegistro,
           pi->nombreOperador);
}


int buscarEnArchivoConIndice(FILE *fp, const tArbol *pa, tOperador *o)

{
    int claveEncontrada;
    tIndice idx;
    memset(&idx, 0, sizeof(idx));
    strncpy(idx.nombreOperador,o->nombre,TAM_NOMBRE);


    claveEncontrada = buscarEnArbol(pa,
                                    &idx,
                                    sizeof(tIndice),
                                    compararClaveIndice);

    if(claveEncontrada == HALLADO) //if(claveEncontrada) --> NUNCA ES FALSO pq buscarEnArbol devuelve 0 o 207
    {
        fseek(fp, idx.indiceRegistro * sizeof(tOperador), SEEK_SET);
        fread(o, sizeof(tOperador), 1, fp);
        return CLAVE_ENCONTRADA;
    }
    else
    {
        return CLAVE_NO_ENCONTRADA;
    }
}

int guardarEnArchivoConIndice(FILE *fp, const tArbol *pa, tOperador *o)

{
    int claveEncontrada;
    tIndice idx;
    memset(&idx, 0, sizeof(idx));
    strncpy(idx.nombreOperador,o->nombre,TAM_NOMBRE);


    claveEncontrada = buscarEnArbol(pa,
                                    &idx,
                                    sizeof(tIndice),
                                    compararClaveIndice);

    if(claveEncontrada == HALLADO) //MISMO CASO que en buscarArcConIndic -->if(claveEncontrada) --> NUNCA ES FALSO pq buscarEnArbol devuelve 0 o 207
    {
        fseek(fp, idx.indiceRegistro * sizeof(tOperador), SEEK_SET);
        fwrite(o, sizeof(tOperador), 1, fp);
        return CLAVE_ENCONTRADA;
    }
    else
    {
        return CLAVE_NO_ENCONTRADA;
    }
}


int indexarArchivoOperador(FILE *fp, tArbol *pa)

{
    tOperador operador;
    tIndice indice;
    memset(&indice, 0, sizeof(indice));
    unsigned long numeroRegistro;

    fread(&operador, sizeof(tOperador), 1, fp);
    numeroRegistro = 0L;
    while(!feof(fp))
    {
        strncpy(indice.nombreOperador, operador.nombre, TAM_NOMBRE);
        indice.indiceRegistro = numeroRegistro;
        insertarArbolBinBusq(pa, &indice, sizeof(tIndice), compararClaveIndice);
        fread(&operador, sizeof(tOperador), 1, fp);
        numeroRegistro ++;
    }

    rewind(fp);
    return ARCHIVO_INDEXADO;
}

/*
void cargarArchivoOrdenadoEnIndiceBalanceado(tArbol *pa,
        FILE *fp,
        int inicio,
        int fin)

{
    size_t medio = (fin+inicio)/2;
    tIndice indice;
    tOperador operador;

    if (inicio > fin)
        return;

    fseek(fp, medio*sizeof(tOperador), SEEK_SET);
    fread(&operador, 1, sizeof(tOperador), fp);
    strncpy(indice.clave.nombreOperador,operador.nombre,TAM_NOMBRE)
    indice.indiceRegistro = medio;

    insertarArbolBinBusq(pa, &indice, sizeof(tIndice), compararClaveIndice);
    cargarArchivoOrdenadoEnIndiceBalanceado(pa, fp, inicio, medio-1);
    cargarArchivoOrdenadoEnIndiceBalanceado(pa, fp, medio+1, fin);
}

int indexarArchivoOperadorOrdenado(const char *nombreArchivo,
                                   tArbol *p)

{
    FILE *fp;
    size_t cantidadRegistros;

    fp = fopen(nombreArchivo, "rb");
    if(!fp)
    {
        printf("No se ha encontrado el archivo \"%s\" para indexarlo\n",
               nombreArchivo);
        return ARCHIVO_NO_INDEXADO;
    }

    fseek(fp, 0, SEEK_END);
    cantidadRegistros = ftell(fp) / sizeof(tOperador);

    cargarArchivoOrdenadoEnIndiceBalanceado(p, fp, 0, cantidadRegistros-1);

    rewind(fp);
    return ARCHIVO_INDEXADO;
}
*/

void mostrarArbolIndiceImpl(const tArbol *p, unsigned nivel)

{
    unsigned i;
    tIndice *info;

    if (!*p)
        return;


    mostrarArbolIndiceImpl(&(*p)->der, nivel + 1);

    for (i = 0; i < nivel; i++)
        printf(" ");

    info = (tIndice *)(*p)->info;
    printf("[Operador: %s] (reg: %u)\n",
           info->nombreOperador,
           info->indiceRegistro);

    mostrarArbolIndiceImpl(&(*p)->izq, nivel + 1);
}

void mostrarArbolIndice(const tArbol *p)

{
    mostrarArbolIndiceImpl(p, 0);
}

void guardarNodoEnArchivo(void *info, unsigned tam, unsigned n, void *params)

{
    FILE *fp = (FILE *)params;
    fwrite(info, tam, 1, fp);
}


int guardarIndiceEnArchivoPreOrden(const tArbol *p, FILE *fp)

{
    recorrerArbolPreOrden(p, 0, fp, guardarNodoEnArchivo);
    return TODO_OK; // Éxito
}

int guardarIndiceEnArchivoInOrden(const tArbol *p, FILE *fp)

{
    recorrerArbolInOrden(p, 0, fp, guardarNodoEnArchivo);
    return TODO_OK; // Éxito
}


int cargarIndiceDesdeArchivo(tArbol *pa, FILE *fp)
{
    tIndice indice;
    int res;

    crearArbol(pa);

    while (fread(&indice, sizeof(tIndice), 1, fp) == 1)
    {
        res = insertarArbolBinBusq(pa,
                                   &indice,
                                   sizeof(tIndice),
                                   compararClaveIndice);

        if (res == SIN_MEM)
        {
            printf("Error: Memoria insuficiente al cargar el indice.\n");
            eliminarArbol(pa);
            rewind(fp);
            return 0;
        }
    }

    rewind(fp);
    return 1;
}
