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
//char* instrucciones = {"iniciar", "leer", "escribir", "entrada-salida", "finalizar"};

void recolectarInstrucciones(char* pathDelArchivoDeInstrucciones) {
	char* contenidoAux;
	char* contenidoDelArchivo;
	FILE* archivoDeInstrucciones = fopen(pathDelArchivoDeInstrucciones, "r");

	contenidoDelArchivo = calloc(ftell(archivoDeInstrucciones) + 1,
			sizeof(char));

	for (; feof(archivoDeInstrucciones) == 0;) {

		fscanf(archivoDeInstrucciones, " %[^\n]", contenidoDelArchivo);

		contenidoAux = calloc(strlen(contenidoDelArchivo) + 1,sizeof(char));

		contenidoAux = string_substring_until(contenidoDelArchivo,strlen(contenidoDelArchivo) - 1);

		contenidoAux[strlen(contenidoDelArchivo)] = '\0';

		separarInstruccionDeParametros(contenidoAux);

	}
	fclose(archivoDeInstrucciones);

	free(contenidoAux);
}

void separarInstruccionDeParametros(char* instruccionMasParametros) {
	//{"iniciar", "leer", "escribir", "entrada-salida", "finalizar"}
	//TODO grabar en lista, crear la estructura...
	int k;
	char** instruccionSpliteada = string_split(instruccionMasParametros, " ");

	for (k = 0; instruccionSpliteada[k] != NULL; k++) {
		printf("contenido: %s \n", instruccionSpliteada[k]);
	}
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
