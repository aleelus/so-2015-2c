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
t_programa* programaEnEjecucion; //Es uno solo por procesador
char* instrucciones[] = { "iniciar", "leer", "escribir", "entrada-salida",
		"finalizar" };

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

t_programa* crearPrograma(char* pathDelArchivoDeInstrucciones) {
	t_programa* programa = malloc(sizeof(t_programa));

	programa->nombre = obtenerNombreDelArchivo(pathDelArchivoDeInstrucciones);
	programa->instrucciones = list_create();

	return programa;
}

t_instruccion* crearInstruccion(char* instruccion) {
	t_instruccion* instr = malloc(sizeof(t_instruccion));
	instr->instruccion = strdup(instruccion);
	instr->parametros = calloc(3,sizeof(char*));
	return instr;
}

void recolectarInstrucciones(char* pathDelArchivoDeInstrucciones) {
	char* contenidoAux;
	char* contenidoDelArchivo;
	FILE* archivoDeInstrucciones = fopen(pathDelArchivoDeInstrucciones, "r");

	programaEnEjecucion = crearPrograma(pathDelArchivoDeInstrucciones);

	contenidoDelArchivo = calloc(ftell(archivoDeInstrucciones) + 1,
			sizeof(char));

	for (; feof(archivoDeInstrucciones) == 0;) {

		fscanf(archivoDeInstrucciones, " %[^\n]", contenidoDelArchivo);

		contenidoAux = calloc(strlen(contenidoDelArchivo) + 1, sizeof(char));

		contenidoAux = string_substring_until(contenidoDelArchivo,
				strlen(contenidoDelArchivo) - 1);

		contenidoAux[strlen(contenidoDelArchivo)] = '\0';

		separarInstruccionDeParametros(contenidoAux);

	}
	fclose(archivoDeInstrucciones);

	free(contenidoAux);
}

void separarInstruccionDeParametros(char* instruccionMasParametros) {
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

	list_add(programaEnEjecucion->instrucciones,instruccion);
}

void ejecutarMProc(char* pathDelArchivoDeInstrucciones) {
	//TODO probablemente esto lo vuele a la shit, lo dejo "por ahora..."!!!
	FILE* archivoDeInstrucciones = fopen(pathDelArchivoDeInstrucciones, "r");
	char* instruccion = string_new();
	char* parametro = string_new();
	char* paquete = string_new();
	char* datosRecibidos = string_new();

	for (; !feof(archivoDeInstrucciones) && !ferror(archivoDeInstrucciones);) {
		fscanf(archivoDeInstrucciones, "%s %s", instruccion, parametro);
		//TODO supongo que todos los programas empiezan con "iniciar cantidadPaginas", pero en el ejemplo de programa, NO pasa esto
		//lo tomo como que se olvidaron de ponerlo, o como si fuera un "iniciar 1" y dsp se setearan mas paginas a medida que necesite?

		/*****************************************************************************************************************************/
		char* auxPaquete0 = "1";		//tipoDeOperacion
		char* auxPaquete1 = obtenerSubBuffer(instruccion);		//ej: iniciar
		char* auxPaquete2 = obtenerSubBuffer(parametro);		//ej: pag 5
		//TODO armar paquete para enviar a la memoria, con el nombre del mProc??? y la cant de pags que necesita...
		string_append(&paquete, auxPaquete0);
		string_append(&paquete, auxPaquete1);
		string_append(&paquete, auxPaquete2);
		/*****************************************************************************************************************************/
		conectarMemoria(&socket_Memoria);
		EnviarDatos(socket_Memoria, paquete, strlen(paquete), YO);

		long unsigned tamDeLosDatosRecibidos = RecibirDatos(socket_Memoria,
				datosRecibidos);
		//la CPU se queda a la espera de si la memoria pudo asignar las paginas al mProc enviado por el planificador para loguear
		//"mProc X iniciado" o "mProc X fallo" (TODO quedar de acuerdo que espera la CPU, agregar en el protocolo)

		break;//avanzo solo una linea, las otras no tienen la misma cantidad de parametros por linea, voy a tener que obtener toda el string de todas las
		//instrucciones y dsp desarmarlo con el string_split o alguno del estilo ya que terminan con ";" TODO
	}
}
