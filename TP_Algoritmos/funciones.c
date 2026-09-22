#include "funciones.h"
#include <ctype.h>
#include <string.h>

#define CANT_COMANDOS 5

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

int validar_operacion( char comando[])
{
    int i=0;
    //char comandoValidos[CANT_COMANDOS][4] = {"DES","REU","ENT","VER", "ESP"};
    const char *comandosValidos[CANT_COMANDOS] = {"DES","REU","ENT","VER", "ESP"};

    for( i;comando[i]!='\0';i++)
    { // Paso a mayuscula
        comando[i] = toupper(comando[i]);
    }

    int c=0;
    for(c;c<CANT_COMANDOS;c++)
    {
        if(strcmp(comando,comandosValidos[c]) == 0 )
        {
            return 1; // Comando existe
        }
    }

    return 0; // Comando erroneo
}
