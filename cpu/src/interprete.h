/*
 * interprete.h
 *
 *  Created on: 1/9/2015
 *      Author: utnso
 */

#ifndef INTERPRETE_H_
#define INTERPRETE_H_

#include <commons/collections/list.h>

typedef struct{
	char* instruccion;
	char** parametros;//vector dinamico de parametros (vector de punteros a char)
	int cantDeParametros;
	char* resultado;//contatenar con barra n, para mandar todo en un char al planificador
}t_instruccion;

typedef struct{
	char* nombre;
	int instructionPointer;//es la pos en la lista de instrucciones de la proxima instruccion a ejecutar;
	int pid;
	//TODO ver que mas datos poner aca, es necesario que la CPU sepa que paginas tiene asignadas o usadas cada proceso?
	t_list* instrucciones;
}t_proceso;

/*
 * @NAME: ejecutarMProc
 * @DESC: ejecuta el mProc leyendo todos las instrucciones del archivo,
 * 		  TODO ver si hacer una funcion que lea todo el archivo de una,
 *  	  grabarlo en una estructura auxiliar y dsp "ejecutar las"
 * */
void ejecutarMProc(char* pathDelArchivoDeInstrucciones, int pid, int ip);

/*
 * @NAME: recolectarInstrucciones
 * @DESC: lee el archivo de instrucciones y separa cada instruccion con sus parametros
 * 		  para grabarlos en una lista
 * */
void recolectarInstrucciones(char* pathDelArchivoDeInstrucciones, int pid);

/*
 * @NAME: separarInstruccionDeParametros
 * @DESC: separa la instruccion de los parametros, los graba en una lista para despues ser ejecutados por ejecutarMProc...
 * */
void separarInstruccionDeParametros(char* instruccionMasParametros, t_proceso* proceso);

/*
 * @NAME: obtenerNombreDelArchivo
 * @DESC: obtiene el nombre de un archivo dado el path Ej: /home/utnso/documentos/cuca.txt ----> retorna "cuca.txt"
 * */
char* obtenerNombreDelArchivo(char* path);

/*
 * @NAME: crearproceso
 * @DESC: crea la estructura t_proceso
 * */
t_proceso* crearProceso(char* pathDelArchivoDeInstrucciones, int pid);

/*
 * @NAME: crearInstruccion
 * @DESC: crea la estructura t_instruccion
 * */
t_instruccion* crearInstruccion(char* instruccion, int cantidadDeParametros);

/*
 * @NAME: ejecutarMCod
 * @DESC: ejecuta la linea de codigo que el ip "dice", manda la instruccion a la memoria y se queda esperando la respuesta para mandar al planificador
 * */
void ejecutarMCod(t_proceso* procesoAEjecutar, int ip);

#endif /* INTERPRETE_H_ */
