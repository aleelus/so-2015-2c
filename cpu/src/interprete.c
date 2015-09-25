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

extern int g_Retardo;//Despues de ejecutar cada instruccion hay que ponerle el retardo
extern char* g_Ip_Memoria;
extern char* g_Puerto_Memoria;
extern __thread int socketPlanificador;

t_proceso* procesoEnEjecucion; //Es uno solo por procesador, TODO me parece que esto esta al pedo...
t_list* procesos;
char* instrucciones[] = { "iniciar", "leer", "escribir", "entrada-salida",
		"finalizar" };

sem_t semListaDeProcesos;

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
	proceso->instructionPointer = 0; //En las listas de las commons la pos 0 es la primera
	proceso->pid = pid;

	return proceso;
}

t_instruccion* crearInstruccion(char* instruccion, int cantParam) {
	t_instruccion* instr = malloc(sizeof(t_instruccion));
	instr->instruccion = strdup(instruccion);
	instr->parametros = calloc(3, sizeof(char*));
	instr->cantDeParametros = cantParam;
	return instr;
}

void recolectarInstrucciones(char* pathDelArchivoDeInstrucciones, int pid) {
	char* contenidoAux;
	char* contenidoDelArchivo;
	FILE* archivoDeInstrucciones = fopen(pathDelArchivoDeInstrucciones, "r");

	t_proceso* proceso = crearProceso(pathDelArchivoDeInstrucciones, pid);

	sem_wait(&semListaDeProcesos);
	list_add(procesos, proceso);
	sem_post(&semListaDeProcesos);

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

int instruccionValida(char* instruccion, int* posicionEnElArray) {
	int i = 0;
	int existeInstruccion = 0;
	int cantInstrucciones = sizeof(instrucciones) / 4;

	for (i = 0; i < cantInstrucciones && !existeInstruccion; i++) {
		//esto en teoria no hace falta ya que "no van a ver instrucciones que no correspondan", pero por las dudas...
		existeInstruccion = strcmp(instruccion, instrucciones[i]) == 0;
		*posicionEnElArray = i;
	}
	return existeInstruccion;
}

void separarInstruccionDeParametros(char* instruccionMasParametros,
		t_proceso* proceso) {
	//{"iniciar", "leer", "escribir", "entrada-salida", "finalizar"}
	int laInstruccionEsEscribir = 0;
	int existeInstruccion = 0;
	int posicionEnElArray = -1;
	int k;
	char* parametro;
	t_instruccion* instruccion;
	char** instruccionSpliteada = string_split(instruccionMasParametros, " ");

	for (k = 0; instruccionSpliteada[k] != NULL; k++) {
		if (k == 0) { //La instruccion siempre va a estar primero
			existeInstruccion = instruccionValida(instruccionSpliteada[k],
					&posicionEnElArray);

			if (!existeInstruccion) {
				puts("Instruccion no valida");
				//la instruccion no existe, romper todo (?, mandar error al planificador de una?. Supuestamente esto nunca va a pasar, pero por las dudas dejo la validacion...
			} else {
				switch (posicionEnElArray) {
				case (0):
					instruccion = crearInstruccion(instrucciones[0], 1);
					break;
				case (1):
					instruccion = crearInstruccion(instrucciones[1], 1);
					break;
				case (2):
					instruccion = crearInstruccion(instrucciones[2], 2);
					laInstruccionEsEscribir = 1;
					break;
				case (3):
					instruccion = crearInstruccion(instrucciones[3], 1);
					break;
				case (4):
					instruccion = crearInstruccion(instrucciones[4], 0);
					break;
				}

			}
		}
		//es un parametro
		parametro = strdup(instruccionSpliteada[k]);

		if(laInstruccionEsEscribir && k == 2){//esto le saca las "" al texto y deja solo el texto
			parametro = string_substring(parametro,1,strlen(parametro)-2);
			laInstruccionEsEscribir = 0;
		}

		instruccion->parametros[k - 1] = parametro; //k>0
	}

	list_add(proceso->instrucciones, instruccion);
}

void ejecutarMProc(char* pathDelArchivoDeInstrucciones, int pid,
		int ip/*linea del archivo a ejecutar*/) {
	//TODO validar que el proceso no haya sido ejecutado antes
	bool _esElProceso(t_proceso* p) {
		return p->pid == pid;
	}

	if (!list_any_satisfy(procesos, (void*) _esElProceso)) {
		//Proceso nuevo
		recolectarInstrucciones(pathDelArchivoDeInstrucciones, pid);
	} else {
		//El proceso ya fue ejecutado antes
	}

	t_proceso* procesoAEjecutar = list_find(procesos, (void*) _esElProceso);

	ejecutarMCod(procesoAEjecutar, ip);
	//TODO enviar a la memoria segun corresponda y ¿bloquerse esperando respuesta para settear la respuesta para dsp darle el mensaje al planificador?
}

void ejecutarMCod(t_proceso* procesoAEjecutar, int ip) {
	int i = ip;
	int posicionEnElArray = -1;
	int existeInstruccion = 0;
	char* bufferRespuesta = string_new();
	int socket_Memoria_Local;

	socket_Memoria_Local = conectarCliente(g_Ip_Memoria, g_Puerto_Memoria);

	for (i = ip; i < procesoAEjecutar->instrucciones->elements_count; i++) { //ejecuta todas las instrucciones, corta con una entrada-salida o finalizar
		t_instruccion* instruccion = list_get(procesoAEjecutar->instrucciones,i);

		existeInstruccion = instruccionValida(instruccion->instruccion,&posicionEnElArray);

		if (0 == posicionEnElArray) { //iniciar

			char* buffer = string_new();
			string_append(&buffer, YO);//ID
			string_append(&buffer, "1"); //Tipo de operacion (1-Nuevo proceso)
			string_append(&buffer, obtenerSubBuffer(string_itoa(procesoAEjecutar->pid)));
			string_append(&buffer,obtenerSubBuffer(instruccion->parametros[0]));

			long unsigned size = strlen(buffer);

			EnviarDatos(socket_Memoria_Local, buffer, size, YO);
			RecibirDatos(socket_Memoria_Local, &bufferRespuesta);

			if(0 == strcmp(bufferRespuesta,"1")){
				//mProc iniciado
				char* resultado = string_new();

				string_append(&resultado,"mProc ");
				string_append(&resultado,string_itoa(procesoAEjecutar->pid));
				string_append(&resultado," ");
				string_append(&resultado,"iniciado");
				string_append(&resultado,"\n");

				instruccion->resultado = resultado;//Me guardo el resultado para despues mandar al planificador
			}else{
				//mProc fallo
				char* resultado = string_new();

				string_append(&resultado,"mProc ");
				string_append(&resultado,string_itoa(procesoAEjecutar->pid));
				string_append(&resultado," ");
				string_append(&resultado,"fallo");
				string_append(&resultado,"\n");

				instruccion->resultado = resultado;//TODO Me guardo el resultado para despues mandar al planificador?
												   //si esta operacion falla, no puedo seguir con las otras.
												   //Mandar acá el resultado al planificador y cortar la ejecucion?
			}


			free(buffer);
			sleep(g_Retardo);//lo pide el enunciado u_u
		}

		if (1 == posicionEnElArray) {//leer

			char* buffer = string_new();

			string_append(&buffer, YO);//ID
			string_append(&buffer,"2");//Tipo de operacion 2-Leer memoria
			string_append(&buffer, obtenerSubBuffer(string_itoa(procesoAEjecutar->pid)));
			string_append(&buffer,obtenerSubBuffer(instruccion->parametros[0]));



			long unsigned size = strlen(buffer);
			EnviarDatos(socket_Memoria_Local, buffer, size, YO);
			RecibirDatos(socket_Memoria_Local, &bufferRespuesta);

			char* contenidoLeido = strdup(bufferRespuesta);

			char* resultado = string_new();
			string_append(&resultado,"mProc ");
			string_append(&resultado,string_itoa(procesoAEjecutar->pid));
			string_append(&resultado," ");
			string_append(&resultado,"pagina ");
			string_append(&resultado,instruccion->parametros[0]);
			string_append(&resultado," ");
			string_append(&resultado,"leida: ");
			string_append(&resultado,contenidoLeido);
			string_append(&resultado,"\n");

			instruccion->resultado = resultado;

			free(buffer);
			sleep(g_Retardo);//lo pide el enunciado u_u
		}

		if (2 == posicionEnElArray) {//escribir

			char* buffer = string_new();

			string_append(&buffer, YO);//ID
			string_append(&buffer,"3");//Tipo de operacion 3-Escribir memoria
			string_append(&buffer, obtenerSubBuffer(string_itoa(procesoAEjecutar->pid)));
			string_append(&buffer,obtenerSubBuffer(instruccion->parametros[0]));//TODO dar vuelta otra ves???
			string_append(&buffer,obtenerSubBuffer(instruccion->parametros[1]));



			long unsigned size = strlen(buffer);
			EnviarDatos(socket_Memoria_Local, buffer, size, YO);
			RecibirDatos(socket_Memoria_Local, &bufferRespuesta);

			//Me llega solo el contenido
			char* contenidoEscrito = strdup(bufferRespuesta);//TODO algo se cambio del lado de la memoria?, ahora la memoria esta retornando el "1" en lugar del "hola" :S
			char* resultado = string_new();

			string_append(&resultado,"mProc ");
			string_append(&resultado,string_itoa(procesoAEjecutar->pid));
			string_append(&resultado," ");
			string_append(&resultado,"pagina ");
			string_append(&resultado,instruccion->parametros[0]);
			string_append(&resultado," ");
			string_append(&resultado,"leida: ");
			string_append(&resultado,contenidoEscrito);//se supone que es lo mismo que el parametros[1]...
			string_append(&resultado,"\n");

			instruccion->resultado = resultado;

			free(buffer);
			sleep(g_Retardo);//lo pide el enunciado u_u
		}

		if (3 == posicionEnElArray) {//entrada-salida
			int k = 0;
			char* resultados = string_new();
			char* respuestaParaElLogDelPlanificador = string_new();

			string_append(&respuestaParaElLogDelPlanificador, YO);//ID
			string_append(&respuestaParaElLogDelPlanificador,"1");//Tipo de operacion 1- Enstrada Salida (Tiempo de E/S, Resultados con barra n)
			string_append(&respuestaParaElLogDelPlanificador, obtenerSubBuffer(string_itoa(procesoAEjecutar->pid)));
			string_append(&respuestaParaElLogDelPlanificador,obtenerSubBuffer(instruccion->parametros[0]));

			for(k=0; k<=i ;k++){//hasta aca se ejecutaron i instrucciones, siendo la numero i la entrada-salida, TODO preguntar al Gallego si lo dejo como "<" o "<="
				t_instruccion* instruccionEjecutada = list_get(procesoAEjecutar->instrucciones,k);

				string_append(&resultados,instruccionEjecutada->resultado);
			}

			string_append(&respuestaParaElLogDelPlanificador,obtenerSubBuffer(resultados));

			//TODO hacer el send al planificador, con el socket que le corresponde a este hilo de CPU + frees
			sleep(g_Retardo);//lo pide el enunciado u_u
			break;//Para de ejecutar!!!!!!!! xD
		}

		if (4 == posicionEnElArray) {//finalizar
			int k = 0;
			char* resultados = string_new();
			char* respuestaParaElLogDelPlanificador = string_new();

			char* buffer = string_new();
			string_append(&buffer,YO);
			string_append(&buffer,"4");//Tipo de operacion 4-Finalizar proceso
			string_append(&buffer,obtenerSubBuffer(string_itoa(procesoAEjecutar->pid)));//PID del proceso que va a morir



			long unsigned size = strlen(buffer);
			EnviarDatos(socket_Memoria_Local, buffer, size, YO);
			RecibirDatos(socket_Memoria_Local, &bufferRespuesta);

			if(0 == strcmp(bufferRespuesta,"1")){//supuestamente no hay razon para que esta operacion falle, pero la memoria me va a mandar un 1 si se hizo todo bien
				char* resultado = string_new();

				string_append(&resultado,"mProc ");
				string_append(&resultado,string_itoa(procesoAEjecutar->pid));
				string_append(&resultado," ");
				string_append(&resultado,"finalizado");

				instruccion->resultado = resultado;
			}else{
				//TODO mandar al planificador todo para la shit?
			}

			for(k=0; k<=i ;k++){//hasta aca se ejecutaron i instrucciones, siendo la numero i finalizar
				t_instruccion* instruccionEjecutada = list_get(procesoAEjecutar->instrucciones,k);

				string_append(&resultados,instruccionEjecutada->resultado);
			}

			string_append(&respuestaParaElLogDelPlanificador, YO);//ID
			string_append(&respuestaParaElLogDelPlanificador,"2");//Tipo de operacion 2- Finalizar (Resultados con barra n)
			string_append(&respuestaParaElLogDelPlanificador, obtenerSubBuffer(string_itoa(procesoAEjecutar->pid)));
			string_append(&respuestaParaElLogDelPlanificador, obtenerSubBuffer(resultados));

			//TODO hacer el send al planificador, con el socket que le corresponde a este hilo de CPU + frees

			sleep(g_Retardo);//lo pide el enunciado u_u
			break;//Harakiri
		}

	}
	close(socket_Memoria_Local);
}
