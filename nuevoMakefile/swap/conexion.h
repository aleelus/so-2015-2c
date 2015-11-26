/*
 * conexion.h
 *
 *  Created on: 5/9/2015
 *      Author: utnso
 */


#ifndef CONEXION_H_
#define CONEXION_H_
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "swap.h"
#include <sys/mman.h>

#define ADM_MEMORIA 3

typedef enum {
	INIT_FAIL,
	INIT_OK
}t_init_result;

typedef enum {
	FIN_FAIL,
	FIN_OK
}t_fin_result;

typedef enum {
	WRITE_FAIL,
	WRITE_OK
}t_write_result;

typedef enum{
	READ_FAIL,
	READ_OK /*Esto no se usa, ya que si leyo bien tiene que devolver el contenido */
}t_read_result;


//METODOS MANEJO SOCKETS
void HiloOrquestadorDeConexiones();
int AtiendeCliente(void * arg);
/*
 * @NAME
 * @DESC: por lo general PID no se usa, en cuyo caso se le envia NULL
 * solo se usara en caso de que soliciten un marco donde fallo va a ser el numero de marco en vez de fallo
 */
void EnviarRespuesta(int operacion, int fallo, int pid);



#endif /* CONEXION_H_ */


