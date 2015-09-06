/*
 * interprete.c
 *
 *  Created on: 1/9/2015
 *      Author: utnso
 */

#include "interprete.h"
#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <api.h>
#include "cpu.h"

extern int socket_Memoria;
t_proceso* procesoEnEjecucion; //Es uno solo por procesador, TODO me parece que esto esta al pedo...
t_list* procesos;
char* instrucciones[] = { "iniciar", "leer", "escribir", "entrada-salida", "finalizar" };

char* obtenerNombreDelArchivo(char* path) {
	char** pathPartido = string_split(path, "/");
	char* nombre;
	int i;
	int posDelNombre;

	for (i = 0; pathPartido[i] != NULL; i++) {
		posDelNombre = i;
	}

	nombre = calloc(strlen(pathPartido[posDelNombre]), sizeof(char));
	nombre = pathPartido[posDelNombre];

	return nombre;
}

t_proceso* crearProceso(char* pathDelArchivoDeInstrucciones, int pid) {
	t_proceso* proceso = malloc(sizeof(t_proceso));

	proceso->nombre = obtenerNombreDelArchivo(pathDelArchivoDeInstrucciones);
	proceso->instrucciones = list_create();
	proceso->instructionPointer = 0;//En las listas de las commons la pos 0 es la primera
	proceso->pid = pid;

	return proceso;
}

t_instruccion* crearInstruccion(char* instruccion) {
	t_instruccion* instr = malloc(sizeof(t_instruccion));
	instr->instruccion = strdup(instruccion);
	instr->parametros = calloc(3,sizeof(char*));
	return instr;
}

void recolectarInstrucciones(char* pathDelArchivoDeInstrucciones, int pid) {
	char* contenidoAux;
	char* contenidoDelArchivo;
	FILE* archivoDeInstrucciones = fopen(pathDelArchivoDeInstrucciones, "r");

	t_proceso* proceso = crearProceso(pathDelArchivoDeInstrucciones, pid);

	list_add(procesos,proceso);

	contenidoDelArchivo = calloc(ftell(archivoDeInstrucciones) + 1,
			sizeof(char));

	for (; feof(archivoDeInstrucciones) == 0;) {

		fscanf(archivoDeInstrucciones, " %[^\n]", contenidoDelArchivo);

		contenidoAux = calloc(strlen(contenidoDelArchivo) + 1, sizeof(char));

		contenidoAux = string_substring_until(contenidoDelArchivo,
				strlen(contenidoDelArchivo) - 1);

		contenidoAux[strlen(contenidoDelArchivo)] = '\0';

		separarInstruccionDeParametros(contenidoAux, proceso);

	}
	fclose(archivoDeInstrucciones);

	free(contenidoAux);
}

void separarInstruccionDeParametros(char* instruccionMasParametros, t_proceso* proceso) {
	//{"iniciar", "leer", "escribir", "entrada-salida", "finalizar"}
	int existeInstruccion = 0;
	int i = 0;
	int posicionEnElArray = -1;
	int cantInstrucciones = sizeof(instrucciones) / 4;
	;
	int k;
	char* parametro;
	t_instruccion* instruccion;
	char** instruccionSpliteada = string_split(instruccionMasParametros, " ");

	for (k = 0; instruccionSpliteada[k] != NULL; k++) {
		if (k == 0) { //la instruccion siempre va a estar primero
			for (i = 0; i < cantInstrucciones && !existeInstruccion; i++) {
				//esto en teoria no hace falta ya que "no van a ver instrucciones que no correspondan", pero por las dudas...
				existeInstruccion = strcmp(instruccionSpliteada[k],
						instrucciones[i]) == 0;
				posicionEnElArray = i;
			}

			if (!existeInstruccion) {
				//TODO la instruccion no existe, romper todo (?, mandar error al planificador de una?
				puts("Instruccion no valida");
			} else {
				switch (posicionEnElArray) {
				case (0):
					instruccion = crearInstruccion(instrucciones[0]);
					instruccion ->parametros[0] = parametro;
					break;
				}
			}
		} else {
			//es un parametro
			parametro = strdup(instruccionSpliteada[k]);
			instruccion ->parametros[k-1] = parametro;//k>0
		}
	}

	list_add(proceso->instrucciones,instruccion);
}

void ejecutarMProc(char* pathDelArchivoDeInstrucciones, int pid) {
	//TODO validar que el proceso no haya sido ejecutado antes
	recolectarInstrucciones(pathDelArchivoDeInstrucciones, pid);
	//TODO settear el proceso en ejecucion o buscarlo en la lista para empezar a ejecutarlo
}
