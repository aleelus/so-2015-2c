/*o
 ============================================================================
 Name        : cpu.c
 Author      : SO didn't C that coming
 Version     : 1.0
 Copyright   : SO didn't C that coming - UTN FRBA 2015
 Description : Trabajo Practivo Sistemas Operativos 2C 2015
 Testing	 :
 ============================================================================
 */

#include <api.h>
#include <commons/log.h>
#include "cpu.h"
#include "interprete.h"

//TODO int id;//lo pide el enunciado, es para diferenciar entre CPU en los logs

//Socket de la Memoria
int socket_Memoria;

//Parametros del archivo de configuracion
char* g_Ip_Planificador;
char* g_Puerto_Planificador;
char* g_Ip_Memoria;
char* g_Puerto_Memoria;
int g_Cantidad_Hilos;
int g_Retardo;
int g_Puerto;

//Contador de Hilos
int cantHilos=0;



//Contador de Hilos
//int cantHilos=0;

// Definimos los hilos principales
pthread_t hCrearHilos;

// - Bandera que controla la ejecución o no del programa. Si está en 0 el programa se cierra.
int g_Ejecutando = 1;

/**********************************************Hariables Globales Para Cada Hilo***********************************************/
sem_t semId;
int id = 0;//para ir asignandole a idCPU

__thread int idCPU = 0;//Esta es una variable global visible solo por un hilo (para cada hilo tiene un valor distinto)
__thread int socketPlanificador;

__thread int esRR = 0;

__thread int quantum;
/******************************************************************************************************************************/

sem_t semPid;
int pid = 0;

extern t_list* procesos;

extern sem_t semListaDeProcesos;

int* socketsDeLosHilosDeCpuDelPlanificador;//cada CPU se va a conectar con el planificador por un socket distinto,
										   //la posicion del vector es = al idCPU

int main(void) {
	sem_init(&semPid,0,1);//esto es para probar varias CPUs sin el planificador

	sem_init(&semId,0,1);

	sem_init(&semListaDeProcesos,0,1);

	//Si el tercer parametro es true graba en archivo y muestra en pantalla sino solo graba en archivo
	logger = log_create(NOMBRE_ARCHIVO_LOG, "cpu", true, LOG_LEVEL_TRACE);

	sem_init(&semDormilon,0,0);

	// Levantamos el archivo de configuracion.
	LevantarConfig();

	inicializarListaDeProcesos();

	inicializarVectorDeSockets();

	int iThreadCrearHilos = pthread_create(&hCrearHilos, NULL,(void*) CrearHilos, NULL );

	if (iThreadCrearHilos) {
		fprintf(stderr,
			"Error al crear hilo - pthread_create() return code: %d\n",
			iThreadCrearHilos);
	}

	pthread_join(hCrearHilos, NULL );

	return 0;
}

void CrearHilos(){
	int i;
	for(i=0;i<g_Cantidad_Hilos;i++){
		CrearCPU();

	}
	sem_wait(&semDormilon);
}

void CrearCPU(){
	//HiloCPU
	pthread_t hHiloCPU;
	printf("Creacion de CPU\n");

	int iThreadCPU = pthread_create(&hHiloCPU, NULL,(void*) ProcesoCPU, NULL );
	if (iThreadCPU) {
		fprintf(stderr,"Error al crear hilo - pthread_create() return code: %d\n",iThreadCPU);
		exit(EXIT_FAILURE);
	}
}

void inicializarListaDeProcesos(){
	procesos = list_create();
}

void inicializarVectorDeSockets(){
	socketsDeLosHilosDeCpuDelPlanificador = calloc(g_Cantidad_Hilos,sizeof(int));
}

void conectarMemoria() {

	//ESTRUCTURA DE SOCKETS; EN ESTE CASO CONECTA CON MEMORIA
	//log_info(logger, "Intentando conectar a memoria\n");
	//conectar con Memoria
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	if (getaddrinfo(g_Ip_Memoria, g_Puerto_Memoria, &hints, &serverInfo) != 0) {// Carga en serverInfo los datos de la conexion
		log_info(logger,
				"ERROR: cargando datos de conexion socket_FS");
	}

	if ((socket_Memoria = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol)) < 0) {
		log_info(logger, "ERROR: crear socket_FS");
	}
	if (connect(socket_Memoria, serverInfo->ai_addr, serverInfo->ai_addrlen)
			< 0) {
		log_info(logger, "ERROR: conectar socket_FS");
	}

	freeaddrinfo(serverInfo);	// No lo necesitamos mas
}

void procesarBuffer(char* buffer, long unsigned tamanioBuffer){

	FILE * archivo = fopen("texto.txt","w");

	fwrite(buffer,sizeof(char),tamanioBuffer,archivo);

	fclose(archivo);

}

void procesarBuffer2(char* buffer, long unsigned tamanioBuffer){

	FILE * archivo = fopen("texto2.txt","w");

	fwrite(buffer,sizeof(char),tamanioBuffer,archivo);

	fclose(archivo);

}

void enviarArchivo(){
	FILE * archivo = fopen("texto.txt","r");

	char * contenido;

	fseek(archivo,0L,SEEK_END);

	long unsigned tamanioA = ftell(archivo);

	contenido = malloc(tamanioA+1);
	memset(contenido,0,tamanioA+1);

	rewind(archivo);

	fread(contenido,1,tamanioA,archivo);

	conectarMemoria();

	EnviarDatos(socket_Memoria,contenido,tamanioA,YO);

	free(contenido);
}

void enviarArchivo2(int socket){
	FILE * archivo = fopen("texto2.txt","r");

	char * contenido;

	fseek(archivo,0L,SEEK_END);

	long unsigned tamanioA = ftell(archivo);

	contenido = malloc(tamanioA+1);
	memset(contenido,0,tamanioA+1);

	rewind(archivo);

	fread(contenido,1,tamanioA,archivo);

	conectarMemoria();

	EnviarDatos(socket,contenido,tamanioA,YO);

	free(contenido);

}

void ProcesoCPU() {

	sem_wait(&semId);
	idCPU = id;
	id++;
	socketPlanificador = conectarCliente(g_Ip_Planificador,g_Puerto_Planificador);
	sem_post(&semId);

	log_info(logger,"INSTANCIA DE CPU CREADA. Id: %d",idCPU);

	escucharPlanificador();

	//Esto de aca abajo es para probar la CPU sola  (comentando las lineas de arriba :P)
	//sem_wait(&semPid);
	//ejecutarMProc("/home/utnso/Documentos/mProg.txt",pid++,0);//esto lo uso para hacer pruebas
	//sem_post(&semPid);

}

void escucharPlanificador(){

	char* buffer = string_new();
	char* bufferRespuesta = string_new();
	int posActual = 2;//para desarmar el mensaje
	int pId;
	int ip;
	int cantInstr;
	char* path;//No hay que hacer malloc aca, lo hace el strdup

	RecibirDatos(socketPlanificador,&buffer);//lalalalalala, esperoooo esperoooo, wiiiiiiiiiiiiiiiiiiii :PPPPP

	bufferRespuesta = DigitosNombreArchivo(buffer,&posActual);
	pId = atoi(bufferRespuesta);

	bufferRespuesta = DigitosNombreArchivo(buffer,&posActual);
	ip = atoi(bufferRespuesta);

	//ip = -1;//Para testear :P

	bufferRespuesta = DigitosNombreArchivo(buffer,&posActual);
	cantInstr = atoi(bufferRespuesta);

	bufferRespuesta = DigitosNombreArchivo(buffer,&posActual);
	path = strdup(bufferRespuesta);

	log_info(logger,"CONTEXTO DE EJECUCION RECIBIDO idCPU: %d PID: %d IP: %d QUANTUM: %d PATH: %s",idCPU, pId, ip, cantInstr, path);

	free(bufferRespuesta);
	free(buffer);

	if(cantInstr == -1){
		//es FIFO!
		ejecutarMProc(path,pId,ip);
	}else{
		//es RR!
		//TODO, es agregar un flag, y contar el quantum para cortar (ejecutar hasta que cantInstr == i (del for de instrucciones ejecutadas))
		esRR = 1;

		quantum = cantInstr;

		ejecutarMProc(path,pId,ip);
	}

}


void ConectarPlanificador(int * socket_Planificador){
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	if (getaddrinfo(g_Ip_Planificador, g_Puerto_Planificador, &hints, &serverInfo) != 0) {// Carga en serverInfo los datos de la conexion
		log_info(logger,
				"ERROR: cargando datos de conexion socket_Planificador");
	}
	if ((*socket_Planificador = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol)) < 0) {
		log_info(logger, "ERROR: crear socket_Planificador");
	}
	if (connect(*socket_Planificador, serverInfo->ai_addr, serverInfo->ai_addrlen)
			< 0) {
		log_info(logger, "ERROR: conectar socket_Planificador");
	}

	freeaddrinfo(serverInfo);	// No lo necesitamos mas
}

#if 1 // METODOS CONFIGURACION //
void LevantarConfig() {
	t_config* config = config_create(PATH_CONFIG);
	// Nos fijamos si el archivo de conf. pudo ser leido y si tiene los parametros
	if (config->properties->table_current_size != 0) {

		// Puerto de escucha
		if (config_has_property(config, "IP_PLANIFICADOR")) {
			g_Ip_Planificador = config_get_string_value(config, "IP_PLANIFICADOR");
		} else
			Error("No se pudo leer el parametro IP_PLANIFICADOR");
		if (config_has_property(config, "PUERTO_PLANIFICADOR")) {
			g_Puerto_Planificador = config_get_string_value(config, "PUERTO_PLANIFICADOR");
		} else
			Error("No se pudo leer el parametro PUERTO_PLANIFICADOR");
		if (config_has_property(config, "IP_MEMORIA")) {
			g_Ip_Memoria = config_get_string_value(config, "IP_MEMORIA");
		} else
			Error("No se pudo leer el parametro IP_MEMORIA");
		if (config_has_property(config, "PUERTO_MEMORIA")) {
			g_Puerto_Memoria = config_get_string_value(config, "PUERTO_MEMORIA");
		} else
			Error("No se pudo leer el parametro PUERTO_MEMORIA");
		if (config_has_property(config, "CANTIDAD_HILOS")) {
			g_Cantidad_Hilos = config_get_int_value(config, "CANTIDAD_HILOS");
		} else
			Error("No se pudo leer el parametro CANTIDAD_HILOS");
		if (config_has_property(config, "RETARDO")) {
			g_Retardo = config_get_int_value(config, "RETARDO");
		} else
			Error("No se pudo leer el parametro RETARDO");
	} else {
		ErrorFatal("No se pudo abrir el archivo de configuracion");
	}
	if (config != NULL ) {
		free(config);
	}
}

#endif
