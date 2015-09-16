/*o
 ============================================================================
 Name        : planificador.c
 Author      : SO didn't C that coming
 Version     : 1.0
 Copyright   : SO didn't C that coming - UTN FRBA 2015
 Description : Trabajo Practivo Sistemas Operativos 2C 2015
 Testing	 :
 ============================================================================
 */

#include "planificador.h"





t_list *PCBs, *colaReady, *colaBloqueados;

pthread_t hOrquestadorConexiones, hConsola;

int cantHilos=0;
int pid=0; // Contador de ID Procesos mProc (semaforo)


int main(void) {

	//Si el tercer parametro es true graba en archivo y muestra en pantalla sino solo graba en archivo
	logger = log_create(NOMBRE_ARCHIVO_LOG, "planificador", false, LOG_LEVEL_TRACE);


	lista_cpu = list_create();

	//CargarListaComandos();

	sem_init(&semListaCpu,0,1);

	// Levantamos el archivo de configuracion.
	LevantarConfig();

	int iThreadConsola;					//Hilo de consola

	//Hilo orquestador conexiones
	int iThreadOrquestador = pthread_create(&hOrquestadorConexiones, NULL, (void*) HiloOrquestadorDeConexiones, NULL );
	if (iThreadOrquestador) {
		fprintf(stderr, "Error al crear hilo - pthread_create() return code: %d\n", iThreadOrquestador);
		exit(EXIT_FAILURE);
	}

	//Este hilo es el que maneja la consola
	if ((iThreadConsola = pthread_create(&hConsola, NULL, (void*) ComenzarConsola, NULL)) != 0){
		fprintf(stderr, "Error al crear hilo - pthread_create() return code: %d\n", iThreadConsola);
		exit(EXIT_FAILURE);
	}

	pthread_join(hOrquestadorConexiones, NULL );
	pthread_join(hConsola, NULL );

	return 0;
}


