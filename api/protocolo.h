/*
 * protocolo.h
 *
 *  Created on: 30/8/2015
 *      Author: utnso
 */

#ifndef PROTOCOLO_H_
#define PROTOCOLO_H_
#define COD_PLANIFICADOR "1"
#define COD_CPU "2"
#define COD_ADM_MEMORIA "3"
#define COD_ADM_SWAP "4"

/******************
 * Cod remitente (ID)

 * 1-Planificador
 * 2-CPU
 * 3-Adm Memoria
 * 4-Adm Swap
 ******************/

/*********************************
 * Adm Memoria: tipos de operacion
 *
 * Remitente
 *
 * 		CPU
 *
 * 		1-Nuevo proceso
 * 		2-Leer memoria
 * 		3-Escribir memoria
 * 		4-Finalizar proceso
 *
 * 		Swap
 *
 * 		1-Respuesta creacion proceso
 * 		2-Contenido marco
 * 		3-Resp marco Grabado
 * 		4-Resp finalizar proceso
 *
 * Destinatario
 *
 * 		CPU
 *
 * 		1-Resp Creacion nuevo proceso
 * 		2-Cibtebudi de la pagina
 * 		3-Ok o contenido
 * 		4-Resp fin proceso
 *
 * 		Swap
 * 		1-Creacion proceso
 * 		2-Solicita marco
 * 		3-Reemplazar marco
 * 		4-Finalizar proceso
 *
 **********************************/

//TODO hacer lo mismo con los otros procesos...

/*
 * CPU
 *
 * Remitente
 *
 * 	Planificador
 *	1-ejecutar el mensaje este va a tener: PID,
 *										   IP (numero de linea de la instruccion a ejecutar),
 *										   Cant instrucciones (FIFO = -1, RR = N (es el Quantum))
 *										   Path del archivo de instrucciones (/home/utnso/documentos/mCod)
 *
 * Destinatario
 *
 * 	Planificador
 *
 * 	1- Enstrada Salida (Tiempo de E/S, Resultados con barra n)
 * 	2- Finalizar (Resultados con barra n)
 * 	3- Quantum (Resultados con barra n)
 * 	4- Fallo (Resultados con barra n ("fallo"))
 * */

#endif /* PROTOCOLO_H_ */
