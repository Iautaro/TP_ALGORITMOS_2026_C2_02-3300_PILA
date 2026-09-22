
#include <stdio.h>
// #include "tdapila.h"
// #include "tdacola.h"
// #include "tdalista.h"
#include "funciones.h"

int main() {
    char comando[4];
    int resultado_comando;
    //printf("Trabajo Practico Puerto de Contenedores - Operacion Contrarreloj\n");
    printf("\nPuerto de Contenedores - Operacion Contrarreloj\n");
    //printf("\nOperador: ");
    //while(temporizador < tiempo_disponible_asignado && condiciones_de_cierre_jornada) O while(tiempo > 0 &&  condiciones_de_cierre_jornada)
    //{

    do{
        mostrar_operaciones(); // Esto sería "Presentacion de operaciones posibles(en pantalla)"
        scanf("%s",&comando); // Esto seria "scanf de operacion (para operador)"
        //printf("El comando es: %s\n");
        resultado_comando = validar_operacion(comando);
    }while( resultado_comando == 0);

    // ejecutar_operacion
    // DES->

    //}

    return 0;
}


/*
PROGRAMA "Operación Contrarreloj" -> Borrador

#define puntuacion (para cada operacion)
	(se utiliza para resumen de ejemplo en pag 14)

Main()
{
	crear variables para resumen (total buques, total contenedores entregados, etc)

	aperturas de archivo config.txt.
	seteo de las configuraciones (creacion de temporizador, setea los minutos para cada operacion)

	apertura y/o creacion de logs de operador y jornada (txt distintos)

	apertura y/o creacion de ranking.

	presentación de consola para operario (logueo)

	iniciar sesión de operador (inicio de operaciones / inicio de temporizador)

	apertura y/o creacion de archivo operadores.txt
	agregar operador si no existe

	apertura de archivo puerto.txt.

	creacion de colas camiones y buques

	iniciar_jornada ()	TEMPORIZADOR = 0 M. (revisar si hay buque, camion en tiempo 0) /// TAREA: MATIAS

	while(temporizador < tiempo_disponible_asignado && condiciones_de_cierre_jornada) O while(tiempo > 0 &&  condiciones_de_cierre_jornada)
	{
		presentación de operaciones posibles (en pantalla)

		scanf de operacion (para operador)

		ejecutar_operacion (puede afectar condiciones_de_cierre_jornada, EJ: entregar puede generar Bloqueo operativo)  TEMPORIZADOR = 0 M. | TEMPORIZADOR = 1 M. /// TAREA: LAUTARO

		if - else para operacion o posible switch (ver eficiencia) /// NO TOCAR ESPERAR A PROXIMA REUNION

			(descuenta tiempo)
			(validar comando)
			(validar tiempo disponible con el que más tiempo ocupe de todas las operaciones)
			(validar existan movimientos disponibles (barcos, camion))
			(actualizar logs de jornada, crea porque es una jornada nueva) /// NO TOCAR ESPERAR A PROXIMA REUNION

		realizar eventos (informar/realiza llegada de barcos, camiones, desembarcos) TEMPORIZADOR = 1 M.  | TEMPORIZADOR = 2 M. /// TAREA: NICOLAS
			(informar/realiza operaciones finalizadas)
			(realiza suma de puntuación por cada operación finalizada)
			(actualiza logs de operador, sobreescribe)
			(actualizar variables de resumen)
	}

	restar puntaje por transportes pendientes. /// TAREA: MATIAS

	actualizar ranking. /// TAREA: MATIAS

	cierre de archivos y liberación de memoria dinamica.
}

TODO LO DEMAS NO MARCADO ARIEL

NOTA:
Los lotes de pruebas deben cubrir casos extremos y simples

*/
