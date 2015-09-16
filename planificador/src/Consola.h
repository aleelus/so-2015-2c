/*
 * Consola.h
 *
 *  Created on: 9/9/2015
 *      Author: utnso
 */

#ifndef CONSOLA_H_
#include "Utilidades/Utils.h"
#define CONSOLA_H_

/***********************GLOBALES************************/

/**********************ESTRUCTURAS**********************/

/************************ENUMS**************************/

typedef enum {
	CPU,
	CORRER,
	PS,
	FINALIZAR,
	HELP,
	COMANDO_ERRONEO
} t_operacion;

/***********************METODOS*************************/
void *ComenzarConsola();
t_operacion obtenerComandoCorrespondiente(char *comando, char *argumento, char **path, int *pid);



#endif /* CONSOLA_H_ */
