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
#include <pthread.h>

pthread_mutex_t semaforoLog = PTHREAD_MUTEX_INITIALIZER;

extern int g_Retardo;//Despues de ejecutar cada instruccion hay que ponerle el retardo
extern char* g_Ip_Memoria;
extern char* g_Puerto_Memoria;
extern __thread int socketPlanificador;
extern __thread int esRR;
extern __thread int quantum;


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

	nombre = strdup(pathPartido[posDelNombre]);

	free(pathPartido);

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
	//instr->parametros = calloc(3, sizeof(char*));//TODO Pasar a dos parametros char* para ver que si se soluciona el error del valgrind
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

	fseek(archivoDeInstrucciones,0L,SEEK_END);

	contenidoDelArchivo = calloc(ftell(archivoDeInstrucciones) + 1,sizeof(char));

	rewind(archivoDeInstrucciones);

	for (; feof(archivoDeInstrucciones) == 0;) {

		fscanf(archivoDeInstrucciones, " %[^\n]", contenidoDelArchivo);

		int length = strlenHastaUnChar(contenidoDelArchivo,';');//Saca el size del contenidoDelArchivo hasta el ;

		contenidoAux = calloc(length,sizeof(char));
		contenidoAux = memcpy(contenidoAux,contenidoDelArchivo,length);

		separarInstruccionDeParametros(contenidoAux, proceso, length);

	}

	if(contenidoDelArchivo != NULL){
		free(contenidoDelArchivo);//TODO este free no estaba, pero me parece que faltaba, pero rompe cuando se ejecuta
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
		t_proceso* proceso, int lengthDeLaInstruccionMasParametros) {
	//{"iniciar", "leer", "escribir", "entrada-salida", "finalizar"}
	int laInstruccionEsEscribir = 0;
	int existeInstruccion = 0;
	int posicionEnElArray = -1;
	int pasePorLoQueSeSuponiaQueIbaACortar = 0;
	int pasePorSacarComillas = 0;
	int k;
	char* parametro;
	char* parametroAux;
	t_instruccion* instruccion;
	char** instruccionSpliteada = string_split(instruccionMasParametros, " ");
	int lengDelTextoDeLaInstruccion;

	//****************************************************************Parche para arreglar el bug de string_split u_u**********************************************************//
	if(0 != strcmp(instruccionSpliteada[0],"finalizar")){
		lengDelTextoDeLaInstruccion = lengthDeLaInstruccionMasParametros - (strlen(instruccionSpliteada[0])/*siempre va a ser escribir*/+ strlen(instruccionSpliteada[1])/*numero*/+ 2/*dos espacios*/);
	}
	//*************************************************************************************************************************************************************************//

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
		if(k != 2){
			parametro = strdup(instruccionSpliteada[k]);
		}

		if(laInstruccionEsEscribir && k == 2){//esto le saca las "" al texto y deja solo el texto
			int z = 0;

			pasePorSacarComillas = 1;

			parametroAux = calloc(lengDelTextoDeLaInstruccion,sizeof(char));
			parametroAux = memcpy(parametroAux,instruccionMasParametros+(2 + strlen(instruccionSpliteada[0]) + strlen(instruccionSpliteada[1])),lengDelTextoDeLaInstruccion);

			parametro = calloc(lengDelTextoDeLaInstruccion,sizeof(char));

			parametro = memcpy(parametro,parametroAux + 1,lengDelTextoDeLaInstruccion - 2);

			//parametro = string_substring(parametroAux,1,strlen(parametroAux)-2);

			laInstruccionEsEscribir = 0;
		}

		if(pasePorSacarComillas){
			free(parametroAux);
			pasePorSacarComillas = 0;
		}

		if(k == 1){
			instruccion->parametro = strdup(parametro); //k>0
		}

		if(k == 2){
			//instruccion->otroParametro = strdup(parametro);//lo que hay que hacer para que valgrind no llore u_u
			instruccion->sizeDelTexto = lengDelTextoDeLaInstruccion - 2;
			instruccion->otroParametro = calloc(lengDelTextoDeLaInstruccion - 2,sizeof(char));
			instruccion->otroParametro = memcpy(instruccion->otroParametro,parametro,lengDelTextoDeLaInstruccion - 2);
		}
		free(parametro);
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

	if(ip == -1){
		//Es la ultima linea
		ip = procesoAEjecutar->instrucciones->elements_count - 1;//le setteo la ultima linea
	}

	ejecutarMCod(procesoAEjecutar, ip);

}

void ejecutarMCod(t_proceso* procesoAEjecutar, int ip) {
	puts("Ejecutando mCod");
	int i = ip;
	int posicionEnElArray = -1;
	int existeInstruccion = 0;
	char* bufferRespuesta /*= string_new()*/;//no hace falta inicializar porque recibirDatos le hace un malloc, en todo caso, recibirDatos no tendria que hacer un realloc?
	int socket_Memoria_Local;

	socket_Memoria_Local = conectarCliente(g_Ip_Memoria, g_Puerto_Memoria);//TODO si no se puede conectar a la memoria--->hilo de CPU se suicida (poner esto como variable de thread)

	for (i = ip; i < procesoAEjecutar->instrucciones->elements_count; i++) { //ejecuta todas las instrucciones, corta con una entrada-salida o finalizar
		t_instruccion* instruccion = list_get(procesoAEjecutar->instrucciones,i);

		existeInstruccion = instruccionValida(instruccion->instruccion,&posicionEnElArray);

		if (0 == posicionEnElArray) { //iniciar
			int boom = 0;//si esto es 1, el planificador tiene que matar el mPcoc

			char* buffer = string_new();
			string_append(&buffer, YO);//ID
			string_append(&buffer, "1"); //Tipo de operacion (1-Nuevo proceso)
			string_append(&buffer, obtenerSubBuffer(string_itoa(procesoAEjecutar->pid)));
			string_append(&buffer,obtenerSubBuffer(instruccion->parametro));

			long unsigned size = strlen(buffer);

			pthread_mutex_lock(&semaforoLog);
			EnviarDatos(socket_Memoria_Local, buffer, size, YO);//TODO comentar el log del enviar datos o poner un semaforo aca... :/
			pthread_mutex_unlock(&semaforoLog);

			pthread_mutex_lock(&semaforoLog);
			RecibirDatos(socket_Memoria_Local, &bufferRespuesta);
			pthread_mutex_unlock(&semaforoLog);

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
				boom = 1;
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

			pthread_mutex_lock(&semaforoLog);
			log_info(logger,"INSTRUCCION: iniciar EJECUTADA PID: %d PARAMETROS: %s RESULTADO: %s",procesoAEjecutar->pid,instruccion->parametro,instruccion->resultado);
			pthread_mutex_unlock(&semaforoLog);

			free(buffer);
			sleep(g_Retardo);//lo pide el enunciado u_u

			if(boom){
				char* respuestaParaElPlanificador = string_new();

				string_append(&respuestaParaElPlanificador, YO);//ID
				string_append(&respuestaParaElPlanificador,"4");//Tipo de operacion 1- Enstrada Salida (CNumero de la ultima linea ejecutada, Tiempo de E/S, Resultados con barra n)
				string_append(&respuestaParaElPlanificador, obtenerSubBuffer(string_itoa(procesoAEjecutar->pid)));

				pthread_mutex_lock(&semaforoLog);
				EnviarDatos(socketPlanificador,respuestaParaElPlanificador,strlen(respuestaParaElPlanificador),YO);
				pthread_mutex_unlock(&semaforoLog);
				boom = 0;

				free(respuestaParaElPlanificador);
				break;//Stop!, ponete a escuchar el planificador!
			}

			if(esRR && quantum == i - ip + 1){
				//3- Quantum (Resultados con barra n)
				char* respuestaParaElPlanificador = string_new();

				string_append(&respuestaParaElPlanificador, YO);//ID
				string_append(&respuestaParaElPlanificador,"3");
				string_append(&respuestaParaElPlanificador, obtenerSubBuffer(string_itoa(procesoAEjecutar->pid)));
				string_append(&respuestaParaElPlanificador, obtenerSubBuffer(string_itoa(i)));
				string_append(&respuestaParaElPlanificador, obtenerSubBuffer(instruccion->resultado));

				pthread_mutex_lock(&semaforoLog);
				EnviarDatos(socketPlanificador,respuestaParaElPlanificador,strlen(respuestaParaElPlanificador),YO);
				pthread_mutex_unlock(&semaforoLog);

				free(respuestaParaElPlanificador);
				break;
			}
		}

		if (1 == posicionEnElArray) {//leer

			char* buffer = string_new();

			string_append(&buffer, YO);//ID
			string_append(&buffer,"2");//Tipo de operacion 2-Leer memoria
			string_append(&buffer, obtenerSubBuffer(string_itoa(procesoAEjecutar->pid)));
			string_append(&buffer,obtenerSubBuffer(instruccion->parametro));



			long unsigned size = strlen(buffer);

			pthread_mutex_lock(&semaforoLog);
			EnviarDatos(socket_Memoria_Local, buffer, size, YO);
			pthread_mutex_unlock(&semaforoLog);

			pthread_mutex_lock(&semaforoLog);
			int sizeDeLaRespuesta = RecibirDatos(socket_Memoria_Local, &bufferRespuesta);
			pthread_mutex_unlock(&semaforoLog);

			//char* contenidoLeido = strdup(bufferRespuesta);//TODO esto no puede estar mas por los barra cero
			char* contenidoLeido = calloc(sizeDeLaRespuesta,sizeof(char));
			contenidoLeido = memcpy(contenidoLeido,bufferRespuesta,sizeDeLaRespuesta);//TODO esto tampoco puede estar ya que el append de abajo appendea mal
			//TODO hacer una funcion que cuente los barra ceros para despues saber de cuanto hacer el malloc de lo de abajo para que no de error de valgrind y se vea lindo en el log
			//TODO esto de aca abajo de esta dando error de valgrind
			/*
			char* contenidoLeido = calloc(sizeDeLaRespuesta*2,sizeof(char));

			int z =0;

			for(z=0; z<sizeDeLaRespuesta ;z++){//lo hago para escribirlo en el log
				if(bufferRespuesta[z]!='\0'){
					strncat(contenidoLeido,&bufferRespuesta[z],1);
				}else{
					strncat(contenidoLeido,"\\0",2);
				}
			}*/

			char* resultado = string_new();
			string_append(&resultado,"mProc ");
			string_append(&resultado,string_itoa(procesoAEjecutar->pid));
			string_append(&resultado," ");
			string_append(&resultado,"pagina ");
			string_append(&resultado,instruccion->parametro);
			string_append(&resultado," ");
			string_append(&resultado,"leida: ");
			string_append(&resultado,contenidoLeido);
			string_append(&resultado,"\n");

			instruccion->resultado = resultado;

			pthread_mutex_lock(&semaforoLog);
			log_info(logger,"INSTRUCCION: leer EJECUTADA PID: %d PARAMETROS: %s RESULTADO: %s",procesoAEjecutar->pid,instruccion->parametro,instruccion->resultado);
			pthread_mutex_unlock(&semaforoLog);

			free(buffer);
			free(contenidoLeido);
			sleep(g_Retardo);//lo pide el enunciado u_u

			if(esRR && quantum == i - ip + 1){
				//3- Quantum (Resultados con barra n)
				int k;
				char* resultados = string_new();
				char* respuestaParaElPlanificador = string_new();

				string_append(&respuestaParaElPlanificador, YO);//ID
				string_append(&respuestaParaElPlanificador,"3");
				string_append(&respuestaParaElPlanificador, obtenerSubBuffer(string_itoa(procesoAEjecutar->pid)));
				string_append(&respuestaParaElPlanificador, obtenerSubBuffer(string_itoa(i)));
				string_append(&respuestaParaElPlanificador, obtenerSubBuffer(instruccion->resultado));

				for(k=ip; k<=i ;k++){//hasta aca se ejecutaron i instrucciones
					t_instruccion* instruccionEjecutada = list_get(procesoAEjecutar->instrucciones,k);

					string_append(&resultados,instruccionEjecutada->resultado);
				}

				string_append(&respuestaParaElPlanificador,obtenerSubBuffer(resultados));

				pthread_mutex_lock(&semaforoLog);
				EnviarDatos(socketPlanificador,respuestaParaElPlanificador,strlen(respuestaParaElPlanificador),YO);
				pthread_mutex_unlock(&semaforoLog);

				free(respuestaParaElPlanificador);
				free(resultados);
				break;
			}
		}

		if (2 == posicionEnElArray) {//escribir

			char* buffer = string_new();

			string_append(&buffer, YO);//ID
			string_append(&buffer,"3");//Tipo de operacion 3-Escribir memoria
			string_append(&buffer, obtenerSubBuffer(string_itoa(procesoAEjecutar->pid)));
			string_append(&buffer,obtenerSubBuffer(instruccion->parametro));//TODO dar vuelta otra ves???

			char* subBuffer = obtenerSubBufferDeContenido(instruccion->otroParametro,instruccion->sizeDelTexto);

			long unsigned size = strlen(buffer) + instruccion->sizeDelTexto + 1 + strlen(string_itoa(instruccion->sizeDelTexto));

			char* bufferMasSubBuffer = calloc(size,sizeof(char));

			memcpy(bufferMasSubBuffer,buffer,strlen(buffer));
			memcpy(bufferMasSubBuffer+strlen(buffer),subBuffer,instruccion->sizeDelTexto + 1 + strlen(string_itoa(instruccion->sizeDelTexto)));

			pthread_mutex_lock(&semaforoLog);
			EnviarDatos(socket_Memoria_Local, bufferMasSubBuffer, size, YO);
			pthread_mutex_unlock(&semaforoLog);

			pthread_mutex_lock(&semaforoLog);
			RecibirDatos(socket_Memoria_Local, &bufferRespuesta);
			pthread_mutex_unlock(&semaforoLog);

			if(0 == strcmp(bufferRespuesta,"0")){
				//Fallo la escritura, esto puede pasar si el tamanio del texto, supera el del marco
				char* respuestaParaElPlanificador = string_new();

				string_append(&respuestaParaElPlanificador, YO);//ID
				string_append(&respuestaParaElPlanificador,"4");//Tipo de operacion Fallo
				string_append(&respuestaParaElPlanificador, obtenerSubBuffer(string_itoa(procesoAEjecutar->pid)));

				pthread_mutex_lock(&semaforoLog);
				EnviarDatos(socketPlanificador,respuestaParaElPlanificador,strlen(respuestaParaElPlanificador),YO);
				pthread_mutex_unlock(&semaforoLog);

				pthread_mutex_lock(&semaforoLog);
				log_info(logger,"INSTRUCCION: escribir FALLO PID: %d PARAMETROS: %s RESULTADO: %s",procesoAEjecutar->pid,instruccion->parametro);
				pthread_mutex_unlock(&semaforoLog);

				free(subBuffer);
				free(respuestaParaElPlanificador);
				free(buffer);
				break;
			}

			//Me llega solo el contenido
			//char* contenidoEscrito = strdup(bufferRespuesta);//TODO algo se cambio del lado de la memoria?, ahora la memoria esta retornando el "1" en lugar del "hola" :S
			char* contenidoEscrito = calloc(instruccion->sizeDelTexto + 2,sizeof(char));

			int z =0;

			for(z=0; z<instruccion->sizeDelTexto ;z++){//lo hago para escribirlo en el log
					if(bufferRespuesta[z]!='\0'){
						strncat(contenidoEscrito,&bufferRespuesta[z],1);
							}else{
						strncat(contenidoEscrito,"\\0",2);
					}
			}

			char* resultado = string_new();

			string_append(&resultado,"mProc ");
			string_append(&resultado,string_itoa(procesoAEjecutar->pid));
			string_append(&resultado," ");
			string_append(&resultado,"pagina ");
			string_append(&resultado,instruccion->parametro);
			string_append(&resultado," ");
			string_append(&resultado,"escrita: ");
			string_append(&resultado,contenidoEscrito);//se supone que es lo mismo que el parametros[1]...
			string_append(&resultado,"\n");

			instruccion->resultado = resultado;

			pthread_mutex_lock(&semaforoLog);
			log_info(logger,"INSTRUCCION: escribir EJECUTADA PID: %d PARAMETROS: %s RESULTADO: %s",procesoAEjecutar->pid,instruccion->parametro,instruccion->resultado);
			pthread_mutex_unlock(&semaforoLog);

			free(bufferMasSubBuffer);
			free(subBuffer);
			free(buffer);
			free(contenidoEscrito);
			sleep(g_Retardo);//lo pide el enunciado u_u

			if(esRR && quantum == i - ip + 1){
				//3- Quantum (Resultados con barra n)
				int k;
				char* resultados = string_new();
				char* respuestaParaElPlanificador = string_new();

				string_append(&respuestaParaElPlanificador, YO);//ID
				string_append(&respuestaParaElPlanificador,"3");
				string_append(&respuestaParaElPlanificador, obtenerSubBuffer(string_itoa(procesoAEjecutar->pid)));
				string_append(&respuestaParaElPlanificador, obtenerSubBuffer(string_itoa(i)));
				string_append(&respuestaParaElPlanificador, obtenerSubBuffer(instruccion->resultado));

				for(k=ip; k<=i ;k++){//hasta aca se ejecutaron i instrucciones
					t_instruccion* instruccionEjecutada = list_get(procesoAEjecutar->instrucciones,k);

					string_append(&resultados,instruccionEjecutada->resultado);
				}

				string_append(&respuestaParaElPlanificador,obtenerSubBuffer(resultados));

				pthread_mutex_lock(&semaforoLog);
				EnviarDatos(socketPlanificador,respuestaParaElPlanificador,strlen(respuestaParaElPlanificador),YO);
				pthread_mutex_unlock(&semaforoLog);

				free(respuestaParaElPlanificador);
				free(resultados);
				break;
			}
		}

		if (3 == posicionEnElArray) {//entrada-salida
			int k = 0;
			char* resultados = string_new();
			char* respuestaParaElLogDelPlanificador = string_new();
			char* resultado = string_new();

			string_append(&respuestaParaElLogDelPlanificador, YO);//ID
			string_append(&respuestaParaElLogDelPlanificador,"1");//Tipo de operacion 1- Enstrada Salida (CNumero de la ultima linea ejecutada, Tiempo de E/S, Resultados con barra n)
			string_append(&respuestaParaElLogDelPlanificador, obtenerSubBuffer(string_itoa(procesoAEjecutar->pid)));
			string_append(&respuestaParaElLogDelPlanificador, obtenerSubBuffer(string_itoa(i)));//Ultima linea ejecutada
			string_append(&respuestaParaElLogDelPlanificador,obtenerSubBuffer(instruccion->parametro));

			string_append(&resultado,"mProc ");
			string_append(&resultado,string_itoa(procesoAEjecutar->pid));
			string_append(&resultado," ");
			string_append(&resultado,"entrada-salida de tiempo: ");
			string_append(&resultado,instruccion->parametro);
			string_append(&resultado,"\n");

			instruccion->resultado = resultado;

			for(k=ip; k<=i ;k++){//hasta aca se ejecutaron i instrucciones, siendo la numero i la entrada-salida, TODO preguntar al Gallego si lo dejo como "<" o "<="
				t_instruccion* instruccionEjecutada = list_get(procesoAEjecutar->instrucciones,k);

				string_append(&resultados,instruccionEjecutada->resultado);
			}

			string_append(&respuestaParaElLogDelPlanificador,obtenerSubBuffer(resultados));

			pthread_mutex_lock(&semaforoLog);
			EnviarDatos(socketPlanificador,respuestaParaElLogDelPlanificador,strlen(respuestaParaElLogDelPlanificador),YO);
			pthread_mutex_unlock(&semaforoLog);

			pthread_mutex_lock(&semaforoLog);
			log_info(logger,"INSTRUCCION: entrada-salida EJECUTADA PID: %d PARAMETROS: %s RESULTADO: %s",procesoAEjecutar->pid,instruccion->parametro,instruccion->resultado);
			pthread_mutex_unlock(&semaforoLog);

			free(resultados);
			free(respuestaParaElLogDelPlanificador);
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

			pthread_mutex_lock(&semaforoLog);
			EnviarDatos(socket_Memoria_Local, buffer, size, YO);
			pthread_mutex_unlock(&semaforoLog);

			pthread_mutex_lock(&semaforoLog);
			RecibirDatos(socket_Memoria_Local, &bufferRespuesta);
			pthread_mutex_unlock(&semaforoLog);

			if(0 == strcmp(bufferRespuesta,"1")){//supuestamente no hay razon para que esta operacion falle, pero la memoria me va a mandar un 1 si se hizo todo bien
				char* resultado = string_new();

				string_append(&resultado,"mProc ");
				string_append(&resultado,string_itoa(procesoAEjecutar->pid));
				string_append(&resultado," ");
				string_append(&resultado,"finalizado");

				instruccion->resultado = resultado;
			}else{
				//TODO mandar al planificador todo para la shit?
				char* resultado = string_new();

				string_append(&resultado,"mProc ");
				string_append(&resultado,string_itoa(procesoAEjecutar->pid));
				string_append(&resultado," ");
				string_append(&resultado,"no se pudo finalizar");

				instruccion->resultado = resultado;
			}

			for(k=ip; k<=i ;k++){//TODO aca esta rompiendo cuando el planificador le manda un -1 ya que las otras instrucciones no tienen respuestas. Agregar un bool para saber si se ejecuto alguna ves?
				t_instruccion* instruccionEjecutada = list_get(procesoAEjecutar->instrucciones,k);

				string_append(&resultados,instruccionEjecutada->resultado);
			}

			string_append(&respuestaParaElLogDelPlanificador, YO);//ID
			string_append(&respuestaParaElLogDelPlanificador,"2");//Tipo de operacion 2- Finalizar (Resultados con barra n)
			string_append(&respuestaParaElLogDelPlanificador, obtenerSubBuffer(string_itoa(procesoAEjecutar->pid)));
			string_append(&respuestaParaElLogDelPlanificador, obtenerSubBuffer(resultados));

			pthread_mutex_lock(&semaforoLog);
			EnviarDatos(socketPlanificador,respuestaParaElLogDelPlanificador,strlen(respuestaParaElLogDelPlanificador),YO);
			pthread_mutex_unlock(&semaforoLog);

			pthread_mutex_lock(&semaforoLog);
			log_info(logger,"INSTRUCCION: finalizar EJECUTADA PID: %d RESULTADO: %s",procesoAEjecutar->pid,instruccion->resultado);
			pthread_mutex_unlock(&semaforoLog);

			free(resultados);
			free(respuestaParaElLogDelPlanificador);
			sleep(g_Retardo);//lo pide el enunciado u_u
			break;//Harakiri TODO paro de ejecutar y a donde voy??, tendria que poner a la CPU a la escucha...
		}

	}
	close(socket_Memoria_Local);
	escucharPlanificador();
}
