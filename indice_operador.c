#include <stdio.h>
#include <string.h>
#include "indice_operador.h"

int compararClaveIndice(const void *a, const void *b) // comparamos por Nombre y Puntaje
{
    const tIndice *idxA = (const tIndice*)a;
    const tIndice *idxB = (const tIndice*)b;

    return strcmp(idxA->nombreOperador, idxB->nombreOperador);
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

    if(claveEncontrada == HALLADO) // buscarEnArbol devuelve HALLADO o NO_HALLADO
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

    if(claveEncontrada == HALLADO)
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
