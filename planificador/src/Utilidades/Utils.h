/*
 * Utils.h
 *
 *  Created on: 9/9/2015
 *      Author: utnso
 */

#ifndef UTILIDADES_UTILS_H_
#define UTILIDADES_UTILS_H_

#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>
#include <netdb.h>
#include <termio.h>
#include <sys/ioctl.h>

/*********COMMONS************/
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <commons/config.h>

/***********API*************/
#include <api.h>







/**********************CONSTANTES******************************/
#define PATH_CONFIG "config.cfg"
#define NOMBRE_ARCHIVO_LOG "planificador.log"
#define STDINFD 0
#define COLOR_VERDE "\x1b[32m"
#define DEFAULT "\x1b[0m"
#define PLANIFICADOR "1"
#define __DEBUG__ 1

const char* estados[];

/***************GLOBALES************************/
sem_t semListaCpu;


t_list *lista_cpu;

t_log *logger; // Logger del commons

int g_Puerto; // Puerto de escucha del planificador
int g_Quantum; // Quantum para RR
//int g_Ejecutando = 1; // Bandera que controla la ejecución o no del programa. Si está en 0 el programa se cierra.
char *g_Algoritmo; // Algoritmo de planificacion FIFO o RR




/*************************ESTRUCTURAS**************************/

typedef struct {
	int socket_Cpu;
	sem_t semaforo;
} t_cpu;


/************************ENUMS********************************/
typedef enum {
	LISTO,
	BLOQUEADO,
	EJECUTANDO,
	ERROR,
	TERMINADO
}t_estado;



/***********************METODOS*******************************/

/* @NAME: CargarListaComandos
 * @DESC: Carga la lista de comandos que acepta la CPU */
int AtiendeCliente(void * arg);
void LevantarConfig();
void procesarBuffer(char* buffer, long unsigned tamanioBuffer);
void HiloOrquestadorDeConexiones();
int cuentaDigitos(int );
void LevantarConfig();
#endif /* UTILIDADES_UTILS_H_ */
