/*
 * interprete.h
 *
 *  Created on: 1/9/2015
 *      Author: utnso
 */

#ifndef INTERPRETE_H_
#define INTERPRETE_H_

typedef struct{//por ahora solo hay parametros de 1 o 2 parametros como maximo,
			  //si no puedo crearme un vector dinamico de chars* o una lista y al carajo
			 //pero mientras tanto... :P
	char* instruccion;
	char* parametro;
	char* otroParametro;
}t_instruccion;

/*
 * @NAME: ejecutarMProc
 * @DESC: ejecuta el mProc leyendo todos las instrucciones del archivo,
 * 		  TODO ver si hacer una funcion que lea todo el archivo de una,
 *  	  grabarlo en una estructura auxiliar y dsp "ejecutar las"
 * */
void ejecutarMProc(char* pathDelArchivoDeInstrucciones);

/*
 * @NAME: recolectarInstrucciones
 * @DESC: lee el archivo de instrucciones y separa cada instruccion con sus parametros
 * 		  para grabarlos en una lista
 * */
void recolectarInstrucciones(char* pathDelArchivoDeInstrucciones);

/*
 * @NAME: separarInstruccionDeParametros
 * @DESC: separa la instruccion de los parametros, los graba en una lista para despues ser ejecutados por ejecutarMProc... TODO re hacer ejecutarMProc
 * */
void separarInstruccionDeParametros(char* instruccionMasParametros);

#endif /* INTERPRETE_H_ */
