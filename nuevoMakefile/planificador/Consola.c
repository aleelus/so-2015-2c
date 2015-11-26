/*
 * Consola.c
 *
 *  Created on: 9/9/2015
 *      Author: utnso
 */
#include "Consola.h"





void *ComenzarConsola(){



	for(;;) {
		char *entrada = (char*)malloc(50); // Comando que ingreso el usuario
		char *argumento = (char*)malloc(200); // Argumento si hubiese
		scanf("%s%[^\n]s", entrada, argumento);

		if (!procesarComando(entrada, argumento)){
			printf("El comando ingresado no es válido, para mostrar los comandos ingrese 'help'\n");
		}

		free(entrada);
		free(argumento);
	}
}

int procesarComando(char *comando, char *argumento){
	char *path;
	int pid;

	switch(obtenerComandoCorrespondiente(comando, argumento, &path, &pid)) {
		case CORRER:
			if(__DEBUG__){
				fprintf(stderr,"Entrando a la funcion CORRER, el PATH es %s\n", path);
			}
			if (!(pid = crearProceso(path))){
				return 0;
				break;
			}
			if (__DEBUG__){
				fprintf(stdout, "Se creo el proceso %s con pid %d\n", path, pid);
			}
			pthread_mutex_lock(&lockLogger);
			log_info(logger, "Se creo el proceso %s con pid %d", path, pid);
			pthread_mutex_unlock(&lockLogger);
			ejecutarDispatcher();
			break;

		case FINALIZAR:
			if(__DEBUG__){
				fprintf(stderr, "Entrando a la funcion FINALIZAR, el PID es %d\n", pid);
			}
			if (finalizarProceso(pid) == NULL){
				fprintf(stdout, "El proceso %d, no existe\n", pid);
				return 1;
				break;
			}
			fprintf(stdout, "El proceso %d se ha finalizado con exito\n", pid);
			ejecutarDispatcher();
			break;

		case CPU:
			if(__DEBUG__){
				fprintf(stderr, "Entrando a la funcion CPU\n");
			}
			mostrarPorcentajesDeUso();
			break;

		case PS:
			if(__DEBUG__){
				fprintf(stderr, "Entrando a la funcion PS\n");
			}
			mostrarProcesos(stdout);
			break;

		case HELP:
			printf("Comandos:\n- CORRER 'PATH': Ejecuta el codigo mAnsisOp ubicado en 'PATH'\n- FINALIZAR 'PID': Finaliza el programa con id = 'PID'\n- PS: Muestra los programas en ejecucion.\n- CPU: Muestra el porcentaje de uso del ultimo minuto de cada CPU\n- HELP: Muestra este mensaje de ayuda\n");
			break;

		default:
			return 0;
			break;
	}
	return 1;
}

t_operacion obtenerComandoCorrespondiente(char *comando, char *argumento, char **path, int *pid){

	/*int tamComando = entrada - strchr(entrada, ' ');
	tamComando = tamComando == entrada ? strlen(entrada) : tamComando; // Si es igual a la entrada es porque es un comando que no tiene espacio

	char *comando = (char*)malloc(tamComando);
	strncpy(comando, entrada, tamComando);*/

	t_operacion operacion;
	if(!strcasecmp(comando, "CORRER")){
		operacion = CORRER;

		*path = argumento+1;//Para eliminarl el espacio en blanco que lee al principio

		pid = NULL;
	}
	else if (!strcasecmp(comando, "FINALIZAR")){

		*pid = strtol(argumento, NULL, 10);
		//*pid = atoi(argumento);

		operacion = (*pid) ? FINALIZAR : COMANDO_ERRONEO; // Si el argumento no es un numero el comando es erroneo

		path = NULL;
	}
	else if (!strcasecmp(comando, "PS")){
		operacion = PS;

		path = NULL;
		pid = NULL;
	}
	else if(!strcasecmp(comando, "CPU")){
		operacion = CPU;

		path = NULL;
		pid = NULL;
	}
	else if (!strcasecmp(comando, "HELP")){
		operacion = HELP;

		path = NULL;
		pid = NULL;
	}
	else {
		operacion = COMANDO_ERRONEO;
		path = NULL;
		pid = NULL;
	}
	return operacion;
}
