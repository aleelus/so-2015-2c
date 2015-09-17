/*
 * Utils.c
 *
 *  Created on: 9/9/2015
 *      Author: utnso
 */
#include "Utils.h"



void HiloOrquestadorDeConexiones() {

	int socket_host;
	struct sockaddr_in client_addr;
	struct sockaddr_in my_addr;
	int yes = 1;
	socklen_t size_addr = 0;

	socket_host = socket(AF_INET, SOCK_STREAM, 0);

	if (socket_host == -1) {
		ErrorFatal("No se pudo inicializar el socket que escucha a los clientes");
	}

	if (setsockopt(socket_host, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		ErrorFatal("Error al hacer el 'setsockopt'");
	}

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(g_Puerto);
	my_addr.sin_addr.s_addr = htons(INADDR_ANY );
	memset(&(my_addr.sin_zero), '\0', 8 * sizeof(char));

	if (bind(socket_host, (struct sockaddr*) &my_addr, sizeof(my_addr)) == -1) {
		ErrorFatal("Error al hacer el Bind. El puerto está en uso");
	}

	if (listen(socket_host, 10) == -1) { // el "10" es el tamaño de la cola de conexiones.
		ErrorFatal("Error al hacer el Listen. No se pudo escuchar en el puerto especificado");
	}

	//Traza("El socket está listo para recibir conexiones. Numero de socket: %d, puerto: %d", socket_host, g_Puerto);
	log_trace(logger, "SOCKET LISTO PARA RECIBIR CONEXIONES. Numero de socket: %d, puerto: %d", socket_host, g_Puerto);

	while (/*g_Ejecutando*/1) {
		int socket_client;

		size_addr = sizeof(struct sockaddr_in);

		if ((socket_client = accept(socket_host,(struct sockaddr *) &client_addr, &size_addr)) != -1) {

			log_trace(logger, "NUEVA CONEXION ENTRANTE. Se ha conectado el cliente (%s) por el puerto (%d). El número de socket del cliente es: %d", inet_ntoa(client_addr.sin_addr), client_addr.sin_port, socket_client);

			// Aca hay que crear un nuevo hilo, que será el encargado de atender al cliente
			pthread_t hNuevoCliente;
			pthread_create(&hNuevoCliente, NULL, (void*) AtiendeCliente, (void *) socket_client);

		}
		else {
			Error("ERROR AL ACEPTAR LA CONEXIÓN DE UN CLIENTE");
		}
	}
	CerrarSocket(socket_host);
}


int AtiendeCliente(void * arg) {

	t_cpu *cpu = malloc(sizeof(t_cpu));

	cpu->socket_Cpu = (int) arg;
	sem_init(&cpu->semaforo,0,0);

	sem_wait(&semListaCpu);
	list_add(lista_cpu,cpu);
	sem_post(&semListaCpu);

	//Bloqueo a la CPU hasta que tenga alguna tarea
	while(1){
		printf("CPU BLOQUEADA\n");
		sem_wait(&cpu->semaforo);
		printf("CPU DESBLOQUEADA\n");

		char* bufferR = string_new();
		long unsigned bytesRecibidos = RecibirDatos(cpu->socket_Cpu,&bufferR);
		procesarBuffer(bufferR,bytesRecibidos);
		free(bufferR);
		//Aca se define el envio de mCod a la cpu para que la procese
	}
	return 1;
}



#if 1 // METODOS CONFIGURACION //
void LevantarConfig() {
	t_config* config = config_create(PATH_CONFIG);
	// Nos fijamos si el archivo de conf. pudo ser leido y si tiene los parametros
	if (config->properties->table_current_size != 0) {

		// Puerto de escucha
		if (config_has_property(config, "PUERTO_ESCUCHA")) {
			g_Puerto = config_get_int_value(config, "PUERTO_ESCUCHA");
		}
		else {
			Error("No se pudo leer el parametro PUERTO_ESCUCHA");
		}


		if (config_has_property(config, "ALGORITMO_PLANIFICACION")) {
			g_Algoritmo = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
		}
		else {
			Error("No se pudo leer el parametro ALGORITMO_PLANIFICACION");
		}


		if (config_has_property(config, "QUANTUM")) {
			g_Quantum = config_get_int_value(config, "QUANTUM");
		}
		else {
			Error("No se pudo leer el parametro PUERTO");
		}


	}
	else {
		ErrorFatal("No se pudo abrir el archivo de configuracion");
	}

	if (config != NULL ) {
		free(config);
	}
}

#endif


char inkey(void) {
  char c;
  struct termio param_ant, params;

  ioctl(STDINFD,TCGETA,&param_ant);

  params = param_ant;
  params.c_lflag &= ~(ICANON|ECHO);
  params.c_cc[4] = 1;

  ioctl(STDINFD,TCSETA,&params);

  fflush(stdin); fflush(stderr); fflush(stdout);
  read(STDINFD,&c,1);

  ioctl(STDINFD,TCSETA,&param_ant);
  return c;
}

int esParecido(char* comando,char* com){
	int i,parecido = 1;
	for(i=0;i<strlen(comando);i++){
		if(comando[i]!=com[i]) parecido = 0;
	}
	return parecido;
}

void enviarArchivo(){

	FILE *archivo = fopen("texto.txt","r");
	t_cpu *cpu;
	char *contenido;

	fseek(archivo,0L,SEEK_END);
	long unsigned tamanioA = ftell(archivo);

	contenido = malloc(tamanioA+1);
	memset(contenido,0,tamanioA+1);

	rewind(archivo);

	fread(contenido,1,tamanioA,archivo);

	cpu = list_get(lista_cpu,0);

	EnviarDatos(cpu->socket_Cpu,contenido,tamanioA, PLANIFICADOR);

	free(contenido);

	sem_post(&cpu->semaforo);
}

void procesarBuffer(char* buffer, long unsigned tamanioBuffer){

	FILE * archivo = fopen("texto2.txt","w");

	fwrite(buffer,sizeof(char),tamanioBuffer,archivo);

	fclose(archivo);
}
