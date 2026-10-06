#include <stdio.h>
#include <string.h>

#define TAM_ID 5            /* "B001" son 4 caracteres + '\0' */
#define MAX_CONT_BUQUE 64
#define TAM_LINEA 256
#define TAM_PILA 256
#define TAM_COLA 256

#define TODO_OK 0
#define ERR_VIGILADOR 201
#define ERR_ARCHIVO 202
#define ERR_LINEA_LARGA 203
#define ERR_MEMORIA 204

#define MIN(a,b) ((a) < (b)? (a):(b))

// -v- A SACAR DE ACÁ -v- TODO - son un placeholder de lo que necesitaba para que funcione el codigo, en cuanto tengamos lo necesario hay que eliminarlo

typedef struct sNodo{
    void*info;
    unsigned tamInfo;
    struct sNodo *sig;
} tNodo;

typedef struct{
    char cola[TAM_COLA];
    unsigned tamDisp;
    unsigned pri;
    unsigned ult;
    struct tNodo * sig;
}tCola;

typedef tNodo* tPila;
typedef tNodo* tLista; // a prim nodo

typedef struct
{
    char ID[TAM_ID];
    int  T;
    tPila PILACAMIONES;     /* contenedores del buque; el primero del archivo queda en el tope */
} INSTRUCCIONBUQUES;

typedef struct
{
    char IdCargamento[TAM_ID];   /* contenedor, ej: C101 */
    int  T;
    char IdCamion[TAM_ID];       /* ej: K001 */
} INSTRUCCIONCAMIONES;

int crearPila(tPila *p)
{
    *p = NULL;
    return TODO_OK;
}

int acolar(tCola *c, const void *dato, unsigned tam)
{
    tNodo *nue = malloc(sizeof(tNodo));

    if(!nue)
        return 0;

    nue->info = malloc(tam);
    if(!nue->info)
    {
        free(nue);
        return 0;
    }

    memcpy(nue->info, dato, tam);
    nue->tamInfo = tam;
    nue->sig = NULL;


//    if(c->ult)
//        c->ult->sig = nue;      /* el anterior último apunta al nuevo */
//    else
//        c->pri = nue;           /* cola vacía: el nuevo es también el primero */


    c->ult = nue;
    return 1;
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

int pilaLlena( const tPila *p, unsigned tam)
{
    tNodo *aux = malloc(sizeof(tNodo));
    void *aux_info = malloc(tam);

    if(aux == NULL || aux_info == NULL)
    {
        free(aux);
        free(aux_info);
        return ERR_MEMORIA; // memoria llena
    }
        free(aux);
        free(aux_info);
        return TODO_OK;// disponible espacio
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

void vaciarPila(tPila *p)
{
    tNodo *nae;

    while(*p)
    {
        nae = *p;
        *p = nae->sig;
        free(nae->info);
        free(nae);
    }
}

// -^- A SACAR DE ACÁ -^- TODO - son un placeholder de lo que necesitaba para que funcione el codigo, en cuanto tengamos lo necesario hay que eliminarlo

/* B001;T=0;C=C101,C102,C103 */
static int parsearBuque(const char *linea, INSTRUCCIONBUQUES *b)
{
    char resto[TAM_LINEA], *cods[MAX_CONT_BUQUE], *tok;
    int n = 0, i;

    if(sscanf(linea, "%4[^;];T=%d;C=%199s", b->ID, &b->T, resto) != 3) // no tocar, creanme que funciona.
        return 0;

    for(tok = strtok(resto, ","); tok && n < MAX_CONT_BUQUE; tok = strtok(NULL, ","))
        cods[n++] = tok;

    if(!crearPila(&b->PILACAMIONES))
        return -1;                                   /* error de memoria */

    for(i = n - 1; i >= 0; i--)                      /* reversa: 1° del archivo = tope */
        if(!apilar(&b->PILACAMIONES, cods[i], strlen(cods[i]) + 1))
        {
            vaciarPila(&b->PILACAMIONES);
            return -1;
        }
    return 1;
}

/* K001;T=4;C=C101 */
static int parsearCamion(const char *linea, INSTRUCCIONCAMIONES *k)
{
    return sscanf(linea, "%4[^;];T=%d;C=%4s", k->IdCamion, &k->T, k->IdCargamento) == 3;
}

int iniciar_jornada(const char *nombreArchivo, int *timer,
                    tCola *colaBuques, tCola *colaCamiones)
{
    enum { CABECERA, S_BUQUES, S_CAMIONES } seccion = CABECERA;
    char linea[TAM_LINEA];
    FILE *pf;

    *timer = 0;

    pf = fopen(nombreArchivo, "rt");
    if(!pf)
        return ERR_ARCHIVO;

    while(fgets(linea, sizeof(linea), pf))
    {
        if(!strchr(linea, '\n') && !feof(pf))        /* línea más larga que el buffer */
        {
            fclose(pf);
            return ERR_LINEA_LARGA;
        }
        linea[strcspn(linea, "\r\n")] = '\0';

        if(*linea == '\0')
            continue;

        if(strcmp(linea, "[BUQUES]") == 0)   { seccion = S_BUQUES;   continue; }
        if(strcmp(linea, "[CAMIONES]") == 0) { seccion = S_CAMIONES; continue; }

        if(seccion == S_BUQUES)
        {
            INSTRUCCIONBUQUES b;
            int r = parsearBuque(linea, &b);

            if(r < 0) { fclose(pf); return ERR_MEMORIA; }
            if(r == 0) continue;                     /* línea mal formada: se ignora */

            if(!acolar(colaBuques, &b, sizeof(b)))
            {
                vaciarPila(&b.PILACAMIONES);
                fclose(pf);
                return ERR_MEMORIA;
            }
  //          if(b.T == 0)
 //               procesarbuque(&b); TODO - cambiar por función que procese los datos
        }
        else if(seccion == S_CAMIONES)
        {
            INSTRUCCIONCAMIONES k;

            if(!parsearCamion(linea, &k))
                continue;

            if(!acolar(colaCamiones, &k, sizeof(k)))
            {
                fclose(pf);
                return ERR_MEMORIA;
            }
//            if(k.T == 0)
//                procesarcamion(&k); TODO - cambiar por función que procese los datos
        }
        /* CABECERA: la consigna no pide procesarla acá, se salta */
    }

    fclose(pf);
    return TODO_OK;
}
