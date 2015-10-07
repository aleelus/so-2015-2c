/*
 * Utils.c
 *
 *  Created on: 9/9/2015
 *      Author: utnso
 */
#include "Utils.h"

const char* estados[] = {"Listo", "Esperando IO", "Bloqueado", "Ejecutando", "Error", "Terminado"};

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

	if (__TEST__){
		int s[3];
		s[0] = 5;
		s[1] = 6;
		s[3] = 7;
		pthread_t hClienteTrucho;
		pthread_create(&hClienteTrucho, NULL, (void*) AtiendeCliente, (void*)s[0]);
		pthread_create(&hClienteTrucho, NULL, (void*) AtiendeCliente, (void*)s[1]);
		pthread_create(&hClienteTrucho, NULL, (void*) AtiendeCliente, (void*)s[2]);
	}
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
	cpu->procesoAsignado = NULL;
	sem_init(&cpu->semaforoMensaje,0,0);
	sem_init(&cpu->semaforoProceso,0,1);
	sem_init(&cpu->semUso,0,1);
	cpu->uso = calloc(1, sizeof(bit_array));
	cpu->horaEntrada = 0;
	cpu->horaSalida = 0;


	sem_wait(&semListaCpu);
	list_add(lista_cpu,cpu);
	sem_post(&semListaCpu);

	log_info(logger, "Se conecto CPU con socket: %d", cpu->socket_Cpu);

	//Bloqueo a la CPU hasta que tenga alguna tarea
	while(1){
		if (__DEBUG__)
			printf("CPU BLOQUEADA\n");
		sem_wait(&cpu->semaforoMensaje);
		if (__DEBUG__)
			printf("CPU DESBLOQUEADA\n");
		char* bufferR = string_new();

		marcarSalidaCpu(cpu);
		if(!(*cpu->uso)){
			Error("Aca hay problemas");
		}
		mostrarPorcentajesDeUso();


		if(__TEST__){
			sleep(10);
			//	CPU	I/O	linea	tiempo	resultado
			//	2	1	114		113		un\nresultado\n
			procesarBuffer(cpu, "21114113213un\nresultado\n", 24 );
			continue;
		}
		long unsigned bytesRecibidos = RecibirDatos(cpu->socket_Cpu,&bufferR);
		procesarBuffer(cpu, bufferR,bytesRecibidos);
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
			char *algoritmo = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
			if (!strcasecmp(algoritmo, "FIFO")){
				g_Algoritmo = FIFO;
			}
			else if(!strcasecmp(algoritmo, "RR")) {
				g_Algoritmo = RR;
			}
			else {
				ErrorFatal("El parametro ALGORITMO_PLANIFICACION no es valido");
			}
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
		config_destroy(config);
		//free(config);
	}
}

#endif

void procesarBuffer(t_cpu *cpu, char* buffer, long unsigned tamanioBuffer){

	/*FILE * archivo = fopen("texto2.txt","w");

	fwrite(buffer,sizeof(char),tamanioBuffer,archivo);

	fclose(archivo);*/
	int posActual = 2;
	char m = buffer[1];
	t_mensaje mensaje = strtol(&m, NULL, 10);
	int proximaInstruccion;
	int pid = strtol(DigitosNombreArchivo(buffer, &posActual), NULL, 10);
	switch (mensaje){
		case ENTRADA_SALIDA:
			proximaInstruccion = strtol(DigitosNombreArchivo(buffer, &posActual), NULL, 10)+1;
			int tiempo = strtol(DigitosNombreArchivo(buffer, &posActual), NULL, 10);
			pasarABloqueados(cpu, tiempo, proximaInstruccion);
			break;
		case FINALIZADO:
			terminarProceso(cpu);
			break;
		case FIN_QUANTUM:
			sem_wait(&cpu->semaforoProceso);
			proximaInstruccion = cpu->procesoAsignado->nroLinea + g_Quantum;
			sem_post(&cpu->semaforoProceso);
			pasarAReady(cpu, proximaInstruccion);
			break;
		case FALLO:
			// TODO: Ver que se hace en caso de fallo si es que existe
			break;
		default:
			Error(logger, "CPU con socket: %d desconectada.", cpu->socket_Cpu);
			break;
	}
	log_info(logger, "Resultado del proceso PID: %d:\n\n%s", pid, DigitosNombreArchivo(buffer, &posActual));
	if(__DEBUG__){
		fprintf(stdout, "Resultado del proceso:\n%s\n", DigitosNombreArchivo(buffer, &posActual));
	}
	ejecutarDispatcher();

}

int enviarMensajeEjecucion(t_cpu cpu) {
	char *mensaje = malloc(3);
	strcpy(mensaje, "11");// Remitente: Planificador, Operacion: Ejecutar proceso
	if(__DEBUG__)
		fprintf(stdout, "Proceso asignado: %d\n", cpu.procesoAsignado->pid);
	string_append(&mensaje, obtenerSubBuffer(string_itoa(cpu.procesoAsignado->pid)));
	if(__DEBUG__)
		fprintf(stdout, "Linea: %d\n", cpu.procesoAsignado->nroLinea);
	string_append(&mensaje, obtenerSubBuffer(string_itoa(cpu.procesoAsignado->nroLinea)));
	if(__DEBUG__)
		fprintf(stdout, "Quantum: %d\n", g_Algoritmo == RR ? g_Quantum : -1);
	string_append(&mensaje, obtenerSubBuffer(string_itoa(g_Algoritmo == RR ? g_Quantum : -1 )));
	if(__DEBUG__)
		fprintf(stdout, "Path: %s\n", cpu.procesoAsignado->path);
	string_append(&mensaje, obtenerSubBuffer(cpu.procesoAsignado->path));
	if(__DEBUG__)
		fprintf(stdout, "Mensaje para la cpu:\n%s\n", mensaje);
	int datosEnviados;
	if (!__TEST__)
		datosEnviados = EnviarDatos(cpu.socket_Cpu, mensaje, strlen(mensaje), PLANIFICADOR);
	else
		datosEnviados = 1;
	free(mensaje);
	return datosEnviados;
}

void marcarEntradaCpu(t_cpu* cpu){
	/*printf("Entrada CPU: \n");
	imprimirBinario(cpu->uso);
	printf("\n");*/
	sem_wait(&cpu->semUso);
	llenarZeros(cpu->uso, (int)(difftime(time(&cpu->horaEntrada), cpu->horaSalida)));
	sem_post(&cpu->semUso);
	/*imprimirBinario(cpu->uso);
	printf("\n");*/
}

void marcarSalidaCpu(t_cpu* cpu){
	/*printf("Salida CPU: \n");
	imprimirBinario(cpu->uso);
	printf("\n");*/
	sem_wait(&cpu->semUso);
	llenarUnos(cpu->uso, (int)(difftime(time(&cpu->horaSalida), cpu->horaEntrada)));
	sem_post(&cpu->semUso);
	/*imprimirBinario(cpu->uso);
	printf("\n");*/
}

void imprimirBinario(bit_array ba){
    if (ba) {
    	imprimirBinario(ba >> 1ULL);
    	putchar((ba & 1ULL) ? '1' : '0');
    }
}

void llenarUnos(bit_array* ba, int n){
	//n = (n>60) ? 60 : n;
	if (n>0){
		*ba <<= n;
		for(;n--;){
			*ba |= 1ULL << n;
		}
	}
    *ba &= mask;
}

void llenarZeros(bit_array* ba, int n){
	//n = (n>60) ? 60 : n;
	if (n>0)
		*ba <<= n;
    *ba &= mask;
}

int sumarBits(bit_array ba){
    if (ba)
    	return (ba & 1ULL) + sumarBits(ba>>1);
    return 0;
}


