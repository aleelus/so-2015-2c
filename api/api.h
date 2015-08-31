/*
 * api.h
 *
 *  Created on: 30/8/2015
 *      Author: utnso
 */

#ifndef API_H_
#define API_H_

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
#include <commons/config.h>
#include <string.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <semaphore.h>
#include <netdb.h>

//Tamaño del buffer
#define BUFFERSIZE 50

// Logger del commons
t_log* logger;

/*
 * @NAME: CerrarSocket
 * @DESC: cierra el socket y escribe en el archivo log
 * */
void CerrarSocket(int socket);

/*
 * @NAME: ErrorFatal
 * @DESC: Escribe el error en el archivo log y termina la ejecucion del proceso
 * */
void ErrorFatal(const char* mensaje, ...);

/*
 * @NAME: ObtenerComandoMSJ
 * @DESC: Hay que obtener el comando dado el buffer. El comando está dado por el primer caracter, que tiene que ser un número.
 * */
int ObtenerComandoMSJ(char* buffer);

/*
 * @NAME: PosicionDeBufferAInt
 * @DESC: TODO
 * */
int PosicionDeBufferAInt(char* buffer, int posicion);

/*
 * @NAME: CartToInt
 * @DESC: TODO
 * */
int ChartToInt(char x);

/*
 * @NAME: RecibirDatos
 * @DESC: recibe TODOS los datos del socket y lo almacena en buffer. Retorna el tam del buffer
 * */
long unsigned RecibirDatos(int socket, char **buffer);

/*
 * @NAME: DigitosNombreArchivo
 * @DESC: TODO
 * */
char* DigitosNombreArchivo(char *buffer,int *posicion);

/*
 * @NAME: EnviarDatos
 * @DESC: Hace el send de los datos del buffer al socket, YO es el char* que para nosotros (Emi y Esme) era ID
 * */
long unsigned EnviarDatos(int socket, char *buffer, long unsigned tamanioBuffer, char* YO);

/*
 * @NAME: obtenerSubBuffer
 * @DESC: Esta funcion recibe un nombre y devuelve ese nombre de acuerdo al protocolo. Ej: carlos ------> 16carlos
 * */
char* obtenerSubBuffer(char *nombre);

/*
 * @NAME: Error
 * @DESC: Hace un print del error (mensaje) y lo escribe en el archivo log
 * */
void Error(const char* mensaje, ...);

#endif /* API_H_ */
