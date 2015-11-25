/*o
 ============================================================================
 Name        : memoria.c
 Author      : SO didn't C that coming
 Version     : 1.0
 Copyright   : SO didn't C that coming - UTN FRBA 2015
 Description : Trabajo Practivo Sistemas Operativos 2C 2015
 Testing	 :
 ============================================================================
 */

#include "memoria.h"

int main(void) {
	//Si el tercer parametro es true graba en archivo y muestra en pantalla sino solo graba en archivo
	logger = log_create(NOMBRE_ARCHIVO_LOG, "Adm de Mem", false,
			LOG_LEVEL_TRACE);

	//Semaforos
	sem_init(&semTLB, 0, 1);
	sem_init(&semMP, 0, 1);
	sem_init(&semLog, 0, 1);
	cantAciertos = 0;
	cantTotalAciertos = 0;
	cantFallos=0;
	//pthread_mutex_unlock(&semSwap);
	//pthread_mutex_lock(&sem);

	// Levantamos el archivo de configuracion.
	LevantarConfig();

	//Adentro de IniciarTLB muestra el seteo del archivo de configuracion
	iniciarTLB();

	iniciarListamProc();

	iniciarMemoriaPrincipal();

	int iThreadSeniales = pthread_create(&hSeniales, NULL, (void*) Seniales,
			NULL);
	if (iThreadSeniales) {
		fprintf(stderr,
				"Error al crear hilo - pthread_create() return code: %d\n",
				iThreadSeniales);
	}

	//Me Conecto a Swap
	if (conexionASwap()) {
		//Hilo orquestador conexiones
		int iThreadOrquestador = pthread_create(&hOrquestadorConexiones, NULL,
				(void*) HiloOrquestadorDeConexiones, NULL);
		if (iThreadOrquestador) {
			fprintf(stderr,
					"Error al crear hilo - pthread_create() return code: %d\n",
					iThreadOrquestador);
			exit(EXIT_FAILURE);
		}
		pthread_join(hOrquestadorConexiones, NULL);
		pthread_join(hSeniales, NULL);
	} else {
		printf("Error, no se pudo conectar a SWAP.\n");
		return -1;
	}
	return 0;
}

void vaciarTLB() {
	t_tlb* tlb;
	int i = 0;

	pthread_mutex_lock(&semTELEBE);
	while (i < list_size(lista_tlb)) {
		tlb = list_get(lista_tlb, i);
		tlb->marco = -1;
		tlb->pagina = -1;
		tlb->pid = -1;
		i++;
	}
	pthread_mutex_unlock(&semTELEBE);

}

void bajarMarcosASwapYLimpiarMP() {
	int i = 0, k = 0;
	t_mProc *mProc;
	t_pagina *pagina;

	while (i < list_size(lista_mProc)) {
		mProc = list_get(lista_mProc, i);

		k = 0;
		while (k < list_size(mProc->paginas)) {
			pagina = list_get(mProc->paginas, k);

			if (pagina->bitModificado == 1) {

				grabarContenidoASwap(mProc->pid, pagina->pagina,
						a_Memoria[pagina->marco].contenido);

			}
			memset(a_Memoria[pagina->marco].contenido, 0, g_Tamanio_Marco);
			a_Memoria[pagina->marco].pag = -1;
			a_Memoria[pagina->marco].pid = -1;
			pagina->bitModificado = 0;
			pagina->bitPuntero = 0;
			pagina->bitUso = 0;
			pagina->bitMP = 0;

			k++;
		}
		i++;
	}

}

int Dump() {

	int status;

	char* cabecera = string_new();
	char* pie = string_new();

	int x = 0;

	//////////////////////////////////////////////////////////////////////
	string_append(&cabecera,
			"\n*********"NEGRITA"Memoria Principal"DEFAULT"*******");

	for (x = 0; x < g_Tamanio_Marco - 5; x++)
		string_append(&cabecera, "*");

	string_append(&cabecera, "\n");
	///////////////////////////////////////////////////////////////////////////

	string_append(&cabecera,
			"* "NEGRITA""COLOR_VERDE"Marco\t Pid\t Contenido\t"DEFAULT"");

	for (x = 0; x < g_Tamanio_Marco - 5; x++)
		string_append(&cabecera, " ");

	string_append(&cabecera, "\n");
	////////////////////////////////////////////////////////////////

	string_append(&cabecera, "*********************************");
	for (x = 0; x < g_Tamanio_Marco - 5; x++)
		string_append(&cabecera, "*");
	string_append(&cabecera, "\n");
	////////////////////////////////////////////////////////////////

	string_append(&pie, "*********************************");

	for (x = 0; x < g_Tamanio_Marco - 5; x++)
		string_append(&pie, "*");
	string_append(&pie, "\n");

	if (!fork()) {

		int j = 0;
		char *contenido = string_new();

		string_append(&contenido, cabecera);

		for (j = 0; j < g_Cantidad_Marcos; j++) {

			string_append(&contenido, "* ");
			string_append(&contenido, string_itoa(j));
			string_append(&contenido, "\t ");
			if (a_Memoria[j].pid < 0)
				string_append(&contenido, "-");
			else
				string_append(&contenido, string_itoa(a_Memoria[j].pid));
			string_append(&contenido, "\t ");
			if (a_Memoria[j].pid < 0)
				string_append(&contenido, "-");
			else
				string_append(&contenido, a_Memoria[j].contenido);
			string_append(&contenido, "\n");
		}

		string_append(&contenido, pie);

		sem_wait(&semLog);
		log_info(logger, contenido);
		sem_post(&semLog);

		free(contenido);

		exit(0);

	} else {

		// ESPERO A Q TERMINE EL HIJO POR AHORA
		wait(&status);

	}

	return 1;
}

void Manejador(int signum) {
	switch (signum) {
	case SIGUSR1:
		printf("* He recibido la señal "COLOR_VERDE""NEGRITA"SIGUSR1\n"DEFAULT);
		pthread_mutex_lock(&semMemPrincipal);
		vaciarTLB();
		pthread_mutex_unlock(&semMemPrincipal);
		break;
	case SIGUSR2:
		printf("* He recibido la señal "COLOR_VERDE""NEGRITA"SIGUSR2\n"DEFAULT);
		pthread_mutex_lock(&semMemPrincipal);
		bajarMarcosASwapYLimpiarMP();
		funcionLimpiarTablasPaginas();
		pthread_mutex_unlock(&semMemPrincipal);
		break;
	case SIGPOLL:
		printf("* He recibido la señal "COLOR_VERDE""NEGRITA"SIGPOLL\n"DEFAULT);

		//Crear un hijo, pasarle por pipe al hijo la MP y el hijo guarda el contenido de toda la memoria a disco
		pthread_mutex_lock(&semMemPrincipal);
		Dump();
		pthread_mutex_unlock(&semMemPrincipal);

		break;
	default:
		printf("Fin de ejecucion\n");
		exit(EXIT_SUCCESS);
	}
}

void Seniales() {
	if (signal(SIGUSR1, Manejador) == SIG_ERR) {
		perror("error en la señal SIGUSR1");
		exit(EXIT_FAILURE);
	}
	if (signal(SIGUSR2, Manejador) == SIG_ERR) {
		perror("error en la señal SIGUSR2");
		exit(EXIT_FAILURE);
	}
	if (signal(SIGPOLL, Manejador) == SIG_ERR) {
		perror("error en SIGTERM");
		exit(EXIT_FAILURE);
	}
	while (1)
		pause();
}

void iniciarListamProc() {

	lista_mProc = list_create();

}

void iniciarMemoriaPrincipal() {
	int i;

	if (g_Cantidad_Marcos > 0) {

		a_Memoria = (t_mp*) malloc(sizeof(t_mp) * g_Cantidad_Marcos);

	}

	if (!strcmp(g_Algoritmo, "LRU")) {

		lista_lru = list_create();

	}
	if (g_Cantidad_Marcos > 0) {

		for (i = 0; i < g_Cantidad_Marcos; i++) {
			a_Memoria[i].pid = -1;
			a_Memoria[i].pag = -1;
			a_Memoria[i].contenido = malloc(g_Tamanio_Marco);

			memset(a_Memoria[i].contenido, 0, g_Tamanio_Marco);
		}

	}
}

void iniciarTLB() {

	printf(COLOR_CYAN""NEGRITA"\t SETEO\n"DEFAULT);

	lista_tlb = list_create();
	if (!strcmp(g_TLB_Habilitada, "SI")) {
		printf("*"COLOR_CYAN""NEGRITA" TLB Habilitada - Entradas:%d\n"DEFAULT,
				g_Entradas_TLB);
		int i;
		for (i = 0; i < g_Entradas_TLB; i++) {
			t_tlb* tlb = malloc(sizeof(t_tlb));
			tlb->pid = -1;
			tlb->marco = -1;
			tlb->pagina = -1;
			list_add(lista_tlb, tlb);
		}
	} else {
		printf("*"COLOR_CYAN" TLB No Habilitada\n"DEFAULT);
	}
	printf("*"COLOR_CYAN""NEGRITA" Algoritmo ultilizado: %s\n"DEFAULT,
			g_Algoritmo);
	printf("*"COLOR_CYAN""NEGRITA" Maximo marcos por proceso: %d\n"DEFAULT,
			g_Maximo_Marcos_Por_Proceso);
	printf("*"COLOR_CYAN""NEGRITA" Cantidad de marcos: %d\n"DEFAULT,
			g_Cantidad_Marcos);
	printf("*"COLOR_CYAN""NEGRITA" Tamaño de marco: %d\n"DEFAULT,
			g_Tamanio_Marco);
	printf("*"COLOR_CYAN""NEGRITA" Retardo: %d\n"DEFAULT, g_Retardo_Memoria);

}

void HiloOrquestadorDeConexiones() {

	int socket_host;
	struct sockaddr_in client_addr;
	struct sockaddr_in my_addr;
	int yes = 1;
	socklen_t size_addr = 0;

	socket_host = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_host == -1)
		ErrorFatal(
				"No se pudo inicializar el socket que escucha a los clientes");

	if (setsockopt(socket_host, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
			== -1) {
		ErrorFatal("Error al hacer el 'setsockopt'");
	}

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(g_Puerto);
	my_addr.sin_addr.s_addr = htons(INADDR_ANY);
	memset(&(my_addr.sin_zero), '\0', 8 * sizeof(char));

	if (bind(socket_host, (struct sockaddr*) &my_addr, sizeof(my_addr)) == -1)
		ErrorFatal("Error al hacer el Bind. El puerto está en uso");

	if (listen(socket_host, 10) == -1) // el "10" es el tamaño de la cola de conexiones.
		ErrorFatal(
				"Error al hacer el Listen. No se pudo escuchar en el puerto especificado");

	//Traza("El socket está listo para recibir conexiones. Numero de socket: %d, puerto: %d", socket_host, g_Puerto);
	sem_wait(&semLog);
	log_trace(logger,
			"SOCKET LISTO PARA RECIBIR CONEXIONES. Numero de socket: %d, puerto: %d",
			socket_host, g_Puerto);
	sem_post(&semLog);

	while (g_Ejecutando) {
		int socket_client;

		size_addr = sizeof(struct sockaddr_in);

		if ((socket_client = accept(socket_host,
				(struct sockaddr *) &client_addr, &size_addr)) != -1) {
			//Traza("Se ha conectado el cliente (%s) por el puerto (%d). El número de socket del cliente es: %d", inet_ntoa(client_addr.sin_addr), client_addr.sin_port, socket_client);
			sem_wait(&semLog);
			log_trace(logger,
					"NUEVA CONEXION ENTRANTE. Se ha conectado el cliente (%s) por el puerto (%d). El número de socket del cliente es: %d",
					inet_ntoa(client_addr.sin_addr), client_addr.sin_port,
					socket_client);
			sem_post(&semLog);
			// Aca hay que crear un nuevo hilo, que será el encargado de atender al cliente
			pthread_t hNuevoCliente;
			pthread_create(&hNuevoCliente, NULL, (void*) AtiendeCliente,
					(void *) socket_client);
		} else {
			Error("ERROR AL ACEPTAR LA CONEXIÓN DE UN CLIENTE");
		}
	}
	CerrarSocket(socket_host);
}

void procesarBuffer(char* buffer, long unsigned tamanioBuffer) {

	FILE * archivo = fopen("texto.txt", "w");

	fwrite(buffer, 1, tamanioBuffer, archivo);

	fclose(archivo);

}

void procesarBuffer2(char* buffer, long unsigned tamanioBuffer) {

	FILE * archivo = fopen("texto2.txt", "w");

	fwrite(buffer, 1, tamanioBuffer, archivo);

	fclose(archivo);

}

void enviarArchivo() {
	FILE * archivo = fopen("texto.txt", "r");

	char * contenido;

	fseek(archivo, 0L, SEEK_END);

	long unsigned tamanioA = ftell(archivo);

	contenido = malloc(tamanioA + 1);
	memset(contenido, 0, tamanioA + 1);

	rewind(archivo);

	fread(contenido, 1, tamanioA, archivo);

	EnviarDatos(socket_Swap, contenido, tamanioA);

	free(contenido);

}

void enviarArchivo2(int socket) {
	FILE * archivo = fopen("texto2.txt", "r");

	char * contenido;

	fseek(archivo, 0L, SEEK_END);

	long unsigned tamanioA = ftell(archivo);

	contenido = malloc(tamanioA + 1);
	memset(contenido, 0, tamanioA + 1);

	rewind(archivo);

	fread(contenido, 1, tamanioA, archivo);

	EnviarDatos(socket, contenido, tamanioA);

	free(contenido);

}

int envioDeInfoIniciarASwap(int pid, int cantidadPaginas) {

	//Hay q verificar la respuesta de Swap para ver si pudo o no Reservar el espacio solicitado

	char * bufferASwap = string_new();
	char * bufferRespuesta = string_new();

	string_append(&bufferASwap, "31");
	string_append(&bufferASwap, obtenerSubBuffer(string_itoa(pid)));
	string_append(&bufferASwap, obtenerSubBuffer(string_itoa(cantidadPaginas)));

	printf(
			"* ("COLOR_VERDE""NEGRITA"Iniciar"DEFAULT") Buffer Enviado a SWAP: %s\n",
			bufferASwap);
	pthread_mutex_lock(&semSwap);
	EnviarDatos(socket_Swap, bufferASwap, strlen(bufferASwap));

	RecibirDatos(socket_Swap, &bufferRespuesta);
	pthread_mutex_unlock(&semSwap);

	printf("* ("COLOR_VERDE""NEGRITA"Iniciar"DEFAULT") Respuesta de swap: %s\n",
			bufferRespuesta);

	if (strcmp(bufferRespuesta, "1") == 0) {

		//El swap pudo reserver el pedido de Inicio de la Cpu
		return 1;

	} else {

		return 0;
	}

}

void implementoIniciarCpu(int socket, char *buffer) {

	//2 1 111 112

	int posActual = 2, pid, cantidadPaginas = 0;
	char *bufferAux, *bufferRespuestaCPU = string_new();
	t_mProc *mProc = malloc(sizeof(t_mProc));

	//Id Proceso
	bufferAux = DigitosNombreArchivo(buffer, &posActual);
	pid = atoi(bufferAux);
	mProc->pid = pid;
	free(bufferAux);

	//Cantidad de Paginas
	bufferAux = DigitosNombreArchivo(buffer, &posActual);
	cantidadPaginas = atoi(bufferAux);

	if (cantidadPaginas <= 0) {
		printf(
				"* ("COLOR_VERDE"Iniciar"DEFAULT") Error en iniciar : cantidad de paginas invalido");
		string_append(&bufferRespuestaCPU, "0");
		EnviarDatos(socket, bufferRespuestaCPU, strlen(bufferRespuestaCPU));

		return;

	}

	mProc->paginas = list_create();
	mProc->cantMarcosPorProceso = 0;
	mProc->totalPaginas = cantidadPaginas;

	printf(
			"********************"NEGRITA"INICIAR"DEFAULT"**************************************\n");
	printf("* CPU solicita iniciar Pid:%d Cantidad de Paginas:%d\n", pid,
			cantidadPaginas);
	//Envio a Swap info necesaria para que reserve el espacio solicitado
	if (envioDeInfoIniciarASwap(pid, cantidadPaginas)) {
		//Agrego nuevo proceso a la lista

		list_add(lista_mProc, mProc);

		string_append(&bufferRespuestaCPU, "1");
		printf(
				"* ("COLOR_VERDE""NEGRITA"Iniciar"DEFAULT") PID:%d y se le reservo en SWAP:%d paginas.\n",
				mProc->pid, cantidadPaginas);

	} else {

		//NO HAY ESPACIO SUFICIENTE EN EL SWAP PARA PODER INICIAR ESE PROCESO
		printf(
				"* NO HAY ESPACIO SUFICIENTE EN EL SWAP PARA INICIAR ESE PROCESO\n");
		string_append(&bufferRespuestaCPU, "0");

	}

	EnviarDatos(socket, bufferRespuestaCPU, strlen(bufferRespuestaCPU));

}

int buscarPaginaEnTLB(int pid, int nroPagina, int *marco) {

	t_tlb *telebe;
	int j = 0;

	pthread_mutex_lock(&semTELEBE);
	if (list_size(lista_tlb) > 0) {
		cantTotalAciertos++;
		for (j = 0; j < g_Entradas_TLB; j++) {
			telebe = list_get(lista_tlb, j);

			if (telebe->pid == pid && telebe->pagina == nroPagina) {
				*marco = telebe->marco;
				cantAciertos++;
				printf(
						"* Pid:%d  -  Pagina:%d se encuentra en TLB  -  Aciertos de la TLB: "COLOR_VERDE""NEGRITA"%d"DEFAULT"/"COLOR_VERDE""NEGRITA"%d"DEFAULT"\n",
						pid, nroPagina, cantAciertos, cantTotalAciertos);
				pthread_mutex_unlock(&semTELEBE);
				return 1;
			}
		}
	}
	printf(
			"* Pid:%d Pagina:%d No se encuentra en TLB  -  Aciertos de la TLB: "COLOR_VERDE""NEGRITA"%d"DEFAULT"/"COLOR_VERDE""NEGRITA"%d"DEFAULT"\n",
			pid, nroPagina, cantAciertos, cantTotalAciertos);
	pthread_mutex_unlock(&semTELEBE);
	return 0;

}

char* buscarEnMemoriaPrincipal(int marco) {

	//Aca falta mas cosas pero no me acuerdo que ajajj

	a_Memoria[marco].marcoEnUso = 1;

	return a_Memoria[marco].contenido;

}

void enviarContenidoACpu(int socket, int pid, int nroPagina, char* contenido,
		int tamanioC) {
	// No se bien por ahora si hace falta el nroPagina pero por las dudas lo mando
	//contenido = cicilianiYeta    pid = 4 nroPagina=3
	//3 2 114 113 213cicilianiYeta

	char *bufferACpu = string_new();

	string_append(&bufferACpu, "32");
	string_append(&bufferACpu, obtenerSubBuffer(string_itoa(pid)));
	string_append(&bufferACpu, obtenerSubBuffer(string_itoa(nroPagina)));
	string_append(&bufferACpu, obtenerSubBuffer(contenido));
	printf("Buffer Enviado a CPU: %s\n", bufferACpu);

	EnviarDatos(socket, bufferACpu, strlen(bufferACpu));
}

int buscarEnTablaDePaginas(int pid, int nroPagina, int *marco) {

	t_mProc *mProc;
	t_pagina *pagina;
	int i = 0, j = 0;

	while (i < list_size(lista_mProc)) {
		mProc = list_get(lista_mProc, i);

		if (mProc->pid == pid) {
			j = 0;
			while (j < list_size(mProc->paginas)) {
				pagina = list_get(mProc->paginas, j);

				if (pagina->pagina == nroPagina && pagina->bitMP == 1) {
					*marco = pagina->marco;

					return 1;

				}
				j++;
			}
		}
		i++;
	}

	return 0;

}

int grabarContenidoASwap(int pid, int nroPagina, char* contenido) {

	//3 3 111 112 14hola

	char * buffer = string_new();
	char * bufferRespuesta = string_new();

	string_append(&buffer, "33");
	printf("* EL PID:%d\n", pid);
	string_append(&buffer, obtenerSubBuffer(string_itoa(pid)));
	string_append(&buffer, obtenerSubBuffer(string_itoa(nroPagina)));
	int digitos = cuentaDigitos(g_Tamanio_Marco);
	//string_append(&buffer,obtenerSubBuffer(contenido));
	string_append(&buffer, string_itoa(digitos));
	string_append(&buffer, string_itoa(g_Tamanio_Marco));
	int tamanio = strlen(buffer);
	buffer = realloc(buffer, tamanio + g_Tamanio_Marco);
	memcpy(buffer + tamanio, contenido, g_Tamanio_Marco);
	printf("* Buffer a Swap ("COLOR_VERDE"Escribir"DEFAULT"):");
	imprimirContenido(buffer, tamanio + g_Tamanio_Marco);
	printf("\n");
	pthread_mutex_lock(&semSwap);
	EnviarDatos(socket_Swap, buffer, tamanio+g_Tamanio_Marco);

	// Aca cuando reciba el buffer con el Contenido me va a venir con el protocolo, tengo q trabajarlo y solo retornar el contenido
	RecibirDatos(socket_Swap, &bufferRespuesta);
	pthread_mutex_unlock(&semSwap);

	if (strcmp(bufferRespuesta, "1") == 0) {

		return 1;

	} else {

		return 0;

	}

}

char * pedirContenidoASwap(int pid, int nroPagina) {

	//3 2 111 112

	char * buffer = string_new();
	char * bufferRespuesta = string_new();

	string_append(&buffer, "32");
	string_append(&buffer, obtenerSubBuffer(string_itoa(pid)));
	string_append(&buffer, obtenerSubBuffer(string_itoa(nroPagina)));

	printf("* ("COLOR_VERDE""NEGRITA"Leer"DEFAULT") Buffer a Swap: %s\n",
			buffer);

	pthread_mutex_lock(&semSwap);

	EnviarDatos(socket_Swap, buffer, strlen(buffer));

	// Aca cuando reciba el buffer con el Contenido me va a venir con el protocolo, tengo q trabajarlo y solo retornar el contenido
	long unsigned tamanio = RecibirDatos(socket_Swap, &bufferRespuesta);

	pthread_mutex_unlock(&semSwap);
	if (tamanio == g_Tamanio_Marco) {
		return bufferRespuesta;
	} else {
		return NULL;
	}
}

void actualizarMemoriaPrincipal(int pid, int nroPagina, char *contenido,
		int tamanioContenido, int marco) {

	t_mProc *mProc;
	t_pagina *pagina;
	int i = 0, j = 0;

	while (i < list_size(lista_mProc)) {
		mProc = list_get(lista_mProc, i);

		if (mProc->pid == pid) {
			j = 0;
			while (j < list_size(mProc->paginas)) {
				pagina = list_get(mProc->paginas, j);
				if (pagina->pagina == nroPagina) {
					pagina->marco = marco;
					pagina->bitMP = 1;
					a_Memoria[pagina->marco].marcoEnUso = 1;
					a_Memoria[pagina->marco].pag = nroPagina;// HAY Q VER ESTA LINEA
					a_Memoria[pagina->marco].pid = pid;
					memcpy(a_Memoria[marco].contenido, contenido,
							tamanioContenido);
					free(contenido);
					break;
				}
				j++;
			}
			break;
		}
		i++;
	}

}

int buscarPagina(t_mProc* mProc, int nroPagina) {

	t_pagina *pagina;
	int i = 0;

	while (i < list_size(mProc->paginas)) {
		pagina = list_get(mProc->paginas, i);

		if (pagina->pagina == nroPagina)
			return i;

		i++;
	}

	return -1;

}

void actualizarTLB(int pid, int nroPagina) {

	t_mProc *mProc;
	t_pagina *pagina;
	t_tlb *telebe;
	int i = 0, totalPaginas = 0, entradasTLB = g_Entradas_TLB, entro = 0;
	int contAgrego = 0, posActual = 0, bandera = 0, contPag = 0;

	while (i < list_size(lista_mProc)) {
		mProc = list_get(lista_mProc, i);
		if (mProc->pid == pid) {
			totalPaginas = list_size(mProc->paginas);

			while (bandera == 0) {

				if (!entro)
					posActual = buscarPagina(mProc, nroPagina);

				if (posActual < totalPaginas) {

					pagina = list_get(mProc->paginas, posActual);

					if (pagina->bitMP == 1 && contAgrego < entradasTLB) {

						pthread_mutex_lock(&semTELEBE);
						telebe = list_get(lista_tlb, contAgrego);

						telebe->pid = pid;
						telebe->pagina = pagina->pagina;
						telebe->marco = pagina->marco;

						pthread_mutex_unlock(&semTELEBE);
						contAgrego++;

					}
					contPag++;

				} else {
					posActual = -1;
				}

				if (contAgrego == entradasTLB || contAgrego == totalPaginas
						|| contPag == totalPaginas)
					bandera = 1;

				posActual++;
				entro = 1;

			}

		}

		i++;
	}

}

void funcionLimpiarTablasPaginas() {
	t_mProc* mProc;
	t_pagina* unaPagina;
	int i = 0, j;

	while (i < list_size(lista_mProc)) {
		mProc = list_get(lista_mProc, i);
		j = 0;
		while (j < list_size(mProc->paginas)) {
			unaPagina = list_get(mProc->paginas, j);
			unaPagina->marco = -1;
			unaPagina->bitMP = 0;
			j++;
		}
		i++;
	}

}

void funcionBuscarPidPagina(int marco, int * pid, int * pagina) {
	t_mProc* mProc;
	t_pagina* unaPagina;

	int i = 0, j;
	*pid = -1;

	while (i < list_size(lista_mProc)) {
		mProc = list_get(lista_mProc, i);
		j = 0;
		while (j < list_size(mProc->paginas)) {
			unaPagina = list_get(mProc->paginas, j);
			//a_Memoria[pagina->marco].pag=nroPagina;// HAY Q VER ESTA LINEA
			if (unaPagina->marco == marco) {
				*pid = mProc->pid;
				*pagina = unaPagina->pagina;
				j = list_size(mProc->paginas);
				i = list_size(lista_mProc);
			}
			j++;
		}
		i++;
	}

	if (*pid == -1) {
		printf("C:%d\n", *pid);
		//abort();
	}
}

void actualizarTablaPagina(int pid, int pagina) {
	t_mProc* mProc;
	t_pagina* unaPagina;

	int i = 0, j;

	while (i < list_size(lista_mProc)) {
		mProc = list_get(lista_mProc, i);
		j = 0;
		if (mProc->pid == pid) {
			while (j < list_size(mProc->paginas)) {
				unaPagina = list_get(mProc->paginas, j);
				//a_Memoria[pagina->marco].pag=nroPagina;// HAY Q VER ESTA LINEA
				if (unaPagina->pagina == pagina) {
					unaPagina->bitMP = 0;
					unaPagina->marco = -1;
					j = list_size(mProc->paginas);
					i = list_size(lista_mProc);
				}
				j++;
			}
		}
		i++;
	}

}

int contarMarcosPorProceso(int pid) {

	t_mProc *mProc;
	int i = 0, aux = 0;

	while (i < list_size(lista_mProc)) {
		mProc = list_get(lista_mProc, i);
		if (mProc->pid == pid) {
			aux = mProc->cantMarcosPorProceso;

			return aux;
		}
		i++;
	}

	return -1;

}

void actualizarCantidadMarcosPorProceso(int pid) {

	t_mProc *mProc;
	int i = 0;

	while (i < list_size(lista_mProc)) {
		mProc = list_get(lista_mProc, i);

		if (mProc->pid == pid) {
			mProc->cantMarcosPorProceso = mProc->cantMarcosPorProceso + 1;

			return;
		}

		i++;
	}

}
/*
 void actualizarPunteroFIFO(int pid,int i){
 int k = 0,entro=0;
 k=i-1;
 while(k>=0 && k<i){
 if(a_Memoria[k].pid==pid){
 a_Memoria[k].bitPuntero=0;
 printf("CAMBIO PUNTERO A 0  ----- PID: %d   MARCO: %d\n",a_Memoria[k].pid,k);
 break;

 }
 k--;
 }

 k=i+1;
 while(k<g_Cantidad_Marcos){
 if(a_Memoria[k].pid==pid){
 a_Memoria[k].bitPuntero = 1;
 printf("CAMBIO PUNTERO A 1  ----- PID: %d   MARCO: %d\n",a_Memoria[k].pid,k);
 entro=1;

 }
 if(!entro) k=0;
 else k++;
 }

 }*/

int preguntarDisponibilidadDeMarcos(int pid) {

	int i = 0, cont = 0, resta = 0;
	int cantMarcosPorProceso = -1;
	cantMarcosPorProceso = contarMarcosPorProceso(pid);

	t_mProc *mProc;

	i = 0;
	while (i < list_size(lista_mProc)) {
		mProc = list_get(lista_mProc, i);
		if (mProc->pid == pid)
			i = list_size(lista_mProc);
		i++;
	}

	resta = mProc->totalPaginas - cantMarcosPorProceso;

	i = 0;
	while (i < g_Cantidad_Marcos && cont < resta
			&& cantMarcosPorProceso < g_Maximo_Marcos_Por_Proceso) {

		if (a_Memoria[i].pag < 0) {
			cont++;
		}
		i++;
	}

	return cont;

}
int dameMarco() {

	int i = 0;

	while (i < g_Cantidad_Marcos) {

		if (a_Memoria[i].pag < 0)
			return i;

		i++;
	}

	return -1;
}

void nuevaPagina(t_mProc *mProc, int nroPagina, int pid, int *marco) {

	t_pagina *auxPagina;

	auxPagina = malloc(sizeof(t_pagina));
	auxPagina->bitMP = 1;
	auxPagina->bitPuntero = 0;
	auxPagina->pagina = nroPagina;
	*marco = dameMarco();
	auxPagina->marco = *marco;

	list_add(mProc->paginas, auxPagina);
	actualizarCantidadMarcosPorProceso(pid);

}

int FIFO2(int *marco, int pid, int nroPagina) {

	int i = 0, k = 0, j = 0, bandera = 0,pos=-1;
	t_mProc *mProc;
	t_pagina *tablaPagina, *auxPagina;

	int pagina, valido;

	while (i < list_size(lista_mProc)) {
		mProc = list_get(lista_mProc, i);

		if (mProc->pid == pid) {

			if (list_size(mProc->paginas) == 0) {

				tablaPagina = malloc(sizeof(t_pagina));
				tablaPagina->bitMP = 1;
				tablaPagina->bitPuntero = 1;
				tablaPagina->pagina = nroPagina;

				if (preguntarDisponibilidadDeMarcos(pid) > 0) {
					*marco = dameMarco();
					tablaPagina->marco = *marco;
					list_add(mProc->paginas, tablaPagina);
					actualizarCantidadMarcosPorProceso(pid);

				} else {
					printf("NO HAY ESPACIO EN LA MP\n");
				}

			} else {

				if (preguntarDisponibilidadDeMarcos(pid) > 0) {

					nuevaPagina(mProc, nroPagina, pid, marco);

				} else {

					k = 0;
					while (k < list_size(mProc->paginas)) {
						tablaPagina = list_get(mProc->paginas, k);

						if (tablaPagina->bitPuntero == 1 && tablaPagina->bitMP == 1) {

							tablaPagina->bitPuntero = 0;
							tablaPagina->bitMP = 0;

							if (buscarPagina(mProc, nroPagina) == -1) {

								nuevaPagina(mProc, nroPagina, pid, marco);

							}

							j=tablaPagina->marco;
							bandera=0;
							while(j<g_Cantidad_Marcos){

								if(a_Memoria[j].pid==pid && a_Memoria[j].pag!=tablaPagina->pagina){

									pos=buscarPagina(mProc,a_Memoria[j].pag);

									auxPagina=list_get(mProc->paginas,pos);
									auxPagina->bitPuntero=1;
									bandera=1;
									j=g_Cantidad_Marcos;

								}
								j++;

								if (j == g_Cantidad_Marcos && bandera == 0) {
									j = 0;
									bandera = 1;
								}
							}

							/*
							if (k == list_size(mProc->paginas) - 1) {
								j = 0;
								while (j < list_size(mProc->paginas)) {
									auxPagina = list_get(mProc->paginas, j);
									if (auxPagina->bitMP == 1) {

										auxPagina->bitPuntero = 1;
										j = list_size(mProc->paginas);
									}

									j++;
								}

							} else {

								j = k + 1;
								while (j < list_size(mProc->paginas)) {
									auxPagina = list_get(mProc->paginas, j);
									if (auxPagina->bitMP == 1) {

										auxPagina->bitPuntero = 1;
										bandera = 1;
										j = list_size(mProc->paginas);
									}

									j++;

									if (j == list_size(mProc->paginas)
											&& bandera == 0) {
										j = 0;
										bandera = 1;
									}

								}

							}
							*/
							pagina = a_Memoria[tablaPagina->marco].pag;
							*marco = tablaPagina->marco;
							valido = grabarContenidoASwap(pid, pagina,
									a_Memoria[*marco].contenido);
							if (valido)
								actualizarTablaPagina(pid, pagina);
							memset(a_Memoria[*marco].contenido, 0,
									g_Tamanio_Marco);

							break;
						}

						k++;
					}

				}
			}
			break;

		}

		i++;
	}

	return 0;

}

t_mProc * buscarPidEnListaMproc(int pid) {
	int j = 0;
	t_mProc *mProc;
	while (j < list_size(lista_mProc)) {
		mProc = list_get(lista_mProc, j);
		if (mProc->pid == pid)
			return mProc;
		j++;
	}

	return NULL;
}

int primeraPasada(int pid, int nroPagina, int *marco) {

	int cantMarcosPorProceso = -1;
	t_mProc *mProc;

	t_lru *lru;
	int i = 0, bandera = 0,cont=0;

	cantMarcosPorProceso = contarMarcosPorProceso(pid);

	/*if(cantMarcosPorProceso>=g_Maximo_Marcos_Por_Proceso){
	 bandera=1;
	 }*/
	i=0;
	while(i<g_Cantidad_Marcos && cantMarcosPorProceso==0){

		if(a_Memoria[i].pag<0){
			cont++;
		}
		if(cont==g_Cantidad_Marcos-1)
			bandera=1;

		i++;
	}

	i = 0;

	while (i < g_Cantidad_Marcos) {

		if (a_Memoria[i].pag < 0
				&& cantMarcosPorProceso < g_Maximo_Marcos_Por_Proceso) {

			mProc = buscarPidEnListaMproc(pid);
			if (mProc != NULL) {
				nuevaPagina(mProc, nroPagina, pid, marco);

			} else {
				printf("ERROR AL BUSCAR EL PID\n");
			}

			return i;
		}
		i++;
	}

	if(bandera == 1){

		return -1;

	}else{

		return -2;

	}

}

void imprimirTablaDePaginas() {

	int i = 0, j = 0;
	t_pagina *pagina;
	t_mProc *mProc;

	printf("*****"NEGRITA"Tabla de Paginas"DEFAULT"****\n");
	printf(
			"* "COLOR_VERDE""NEGRITA"Marco"DEFAULT"\t"COLOR_VERDE""NEGRITA"Pid"DEFAULT"\t"COLOR_VERDE""NEGRITA"Pag"DEFAULT"\t*\n");
	printf("**************************\n");
	while (i < list_size(lista_mProc)) {
		mProc = list_get(lista_mProc, i);

		j = 0;
		while (j < list_size(mProc->paginas)) {
			pagina = list_get(mProc->paginas, j);
			printf("*  %d\t%d\t%d\t*\n", pagina->marco, mProc->pid,
					pagina->pagina);
			j++;
		}
		i++;
	}
	printf("*************************\n");

}

void LRU(int *marco, int pid, int nroPagina, char *contenido) {

	//ASIGNACION FIJA Y REEMPLAZO LOCAL

	int i = 0, j = 0, k = 0, contLRU = 0, valido = -1;
	int cont = 0, maximo = -1;
	int marquito, pag = -1;
	t_lru *lru;
	t_mProc *mProc, *copiaMproc;
	t_pagina *pagina;
	int encontre=0;

	lru = malloc(sizeof(t_lru));
	lru->pagina = nroPagina;
	lru->pid = pid;
	list_add(lista_lru, lru);



	marquito = primeraPasada(pid, nroPagina, marco);

//	printf("MARQUITO : %d \n",marquito);

	if (marquito == -2) {

		while (i < list_size(lista_mProc)) {
			mProc = list_get(lista_mProc, i);

			if (mProc->pid == pid) {
				j = 0;
				while (j < list_size(mProc->paginas)) {
					pagina = list_get(mProc->paginas, j);

					cont = 0;
					contLRU = 0;

					if (pagina->bitMP == 1) {
						for (k = list_size(lista_lru) - 2; k >= 0; k--) {

							if (contLRU <= g_Cantidad_Marcos - 1) {
								lru = list_get(lista_lru, k);
								if (lru->pid == pid	&& a_Memoria[pagina->marco].pid	== pid) {

									if (lru->pagina != a_Memoria[pagina->marco].pag) {
										cont++;
									}else{
										k=-1;
									}

								}
							}

							contLRU++;

						}
						if (cont > maximo) {

							maximo = cont;
							pag = pagina->pagina;
							*marco = pagina->marco;



						}
					}

					j++;
				}
				copiaMproc = mProc;
			}
			i++;
		}

		i=0;
		while(i<list_size(lista_mProc)){
			mProc=list_get(lista_mProc,i);
			if(mProc->pid==pid){
				j=0;
				while(j<list_size(mProc->paginas)){
					pagina=list_get(mProc->paginas,j);

					if(pagina->pagina==nroPagina){
						encontre=1;
					}
					j++;
				}
			}
			i++;
		}

		if(encontre==0){
			pagina = malloc(sizeof(t_pagina));
			pagina->bitMP = 1;
			pagina->bitPuntero = 0;
			pagina->pagina = nroPagina;
			pagina->marco = *marco;
			list_add(copiaMproc->paginas, pagina);
			actualizarCantidadMarcosPorProceso(pid);
		}
		valido = grabarContenidoASwap(pid, pag, a_Memoria[*marco].contenido);
		if (valido)
			actualizarTablaPagina(pid, pag);
		memset(a_Memoria[*marco].contenido, 0, g_Tamanio_Marco);

		memcpy(a_Memoria[*marco].contenido, contenido, g_Tamanio_Marco);
		a_Memoria[*marco].pag = nroPagina;

		a_Memoria[*marco].pid = pid;

		if (list_size(lista_lru) >= g_Cantidad_Marcos) {

			lru = list_remove(lista_lru, 0);
			free(lru);

			printf("ELIMINO Y AGREGO DE LA LISTA LRU\n");


		}

	}else if(marquito == -1){
		*marco =-1;

	}

}

t_pagina* buscarDatosEnTP(int marco) {

	int i = 0, k = 0;
	t_mProc *mProc;
	t_pagina *pagina;

	while (i < list_size(lista_mProc)) {
		mProc = list_get(lista_mProc, i);

		k = 0;
		while (k < list_size(mProc->paginas)) {
			pagina = list_get(mProc->paginas, k);

			if (pagina->marco == marco && pagina->bitMP == 1) {
				return pagina;
			}

			k++;
		}

		i++;
	}

	return NULL;

}

int damePuntero(t_list* paginas) {
	int j = 0;
	t_pagina *pagina;
	while (j < list_size(paginas)) {
		pagina = list_get(paginas, j);
		if (pagina->bitPuntero == 1)
			return j;
		j++;
	}
	return -1;
}

int damePaginaVictimaClockMejorado(int pid, int* marco) {
	t_mProc *mProc;
	t_pagina *pagina;
	int i = 0, j = 0, cantPag, encontrado = 0, victima;

	while (i < list_size(lista_mProc)) {
		mProc = list_get(lista_mProc, i);
		if (mProc->pid == pid) {
			cantPag = list_size(mProc->paginas);

			j = damePuntero(mProc->paginas);
			if (j > -1) {
				while (j < cantPag && !encontrado) {
					pagina = list_get(mProc->paginas, j);
					if (pagina->bitUso == 0 && pagina->bitModificado == 0) {
						encontrado = 1;
						victima = pagina->pagina;
						*marco = pagina->marco;
					}
					j++;
				}

				if (encontrado) {
					pagina->bitPuntero = 0;
					j++;
					if (j < cantPag) {
						pagina = list_get(mProc->paginas, j);
						pagina->bitPuntero = 1;
					} else {
						pagina = list_get(mProc->paginas, 0);
						pagina->bitPuntero = 1;
					}
					return victima;
				} else {
					j = 0;
					while (j < cantPag && !encontrado) {
						pagina = list_get(mProc->paginas, j);
						if (pagina->bitUso == 0 && pagina->bitModificado == 1) {
							encontrado = 1;
							victima = pagina->pagina;
							*marco = pagina->marco;
						}
						j++;
					}
					if (encontrado) {
						pagina->bitPuntero = 0;
						j++;
						if (j < cantPag) {
							pagina = list_get(mProc->paginas, j);
							pagina->bitPuntero = 1;
						} else {
							pagina = list_get(mProc->paginas, 0);
							pagina->bitPuntero = 1;
						}
						return victima;
					} else {
						while (j < cantPag && !encontrado) {
							pagina = list_get(mProc->paginas, j);
							if (pagina->bitUso == 0
									&& pagina->bitModificado == 0) {
								encontrado = 1;
								victima = pagina->pagina;
								*marco = pagina->marco;
							}
							j++;
						}
						pagina->bitPuntero = 0;
						j++;
						if (j < cantPag) {
							pagina = list_get(mProc->paginas, j);
							pagina->bitPuntero = 1;
						} else {
							pagina = list_get(mProc->paginas, 0);
							pagina->bitPuntero = 1;
						}
						return victima;
					}
				}
			} else {
				printf("NO HAY PUNTERO MAL:%d\n", pid);
				abort();
			}
		}
		i++;
	}

	return -1;

}

void imprimirMemoria() {
	int i = 0;
	t_pagina *pagina;
	t_lru *lru;

	if (!strcmp(g_Algoritmo, "FIFO") || !strcmp(g_Algoritmo, "LRU")) {

		printf("********"NEGRITA"Memoria Principal"DEFAULT"********\n");
		printf(
				"* "COLOR_VERDE""NEGRITA"Marco"DEFAULT"\t"COLOR_VERDE""NEGRITA"Pid"DEFAULT"\t"COLOR_VERDE""NEGRITA"Pag"DEFAULT"\t"COLOR_VERDE""NEGRITA"Punt"DEFAULT"\t*\n");
		printf("*********************************\n");
		while (i < g_Cantidad_Marcos) {

			if (a_Memoria[i].pag >= 0) {
				pagina = buscarDatosEnTP(i);
				if (pagina != NULL)
					printf("*  %d\t%d\t%d\t%d\t*\n", i, a_Memoria[i].pid,
							a_Memoria[i].pag, pagina->bitPuntero);
			}
			i++;
		}
		printf("*********************************\n");

		/*
		 printf("|MARCO-PID-PAGINA-PUNTERO|\n");
		 while(i<g_Cantidad_Marcos){

		 if(a_Memoria[i].pag>=0){
		 pagina = buscarDatosEnTP(i);
		 if(pagina!=NULL)
		 printf("|%d-%d-%d-%d|",i,a_Memoria[i].pid,a_Memoria[i].pag,pagina->bitPuntero);
		 }

		 i++;
		 }
		 printf("\n");*/
	}

	if (!strcmp(g_Algoritmo, "CLOCKMEJORADO")) {
		printf("|MARCO-PID-PAGINA-PUNTERO-BITUSO|BITMODIFICADO|\n");
		while (i < g_Cantidad_Marcos) {

			if (a_Memoria[i].pag >= 0) {
				pagina = buscarDatosEnTP(i);
				if (pagina != NULL)
					printf("|%d-%d-%d-%d-%d-%d|", i, a_Memoria[i].pid,
							a_Memoria[i].pag, pagina->bitPuntero,
							pagina->bitUso, pagina->bitModificado);
			}

			i++;
		}
		printf("\n");
	}

	if (!strcmp(g_Algoritmo, "CLOCK")) {
		printf("|MARCO-PID-PAGINA-PUNTERO-BITUSO|\n");
		while (i < g_Cantidad_Marcos) {

			if (a_Memoria[i].pag >= 0) {
				pagina = buscarDatosEnTP(i);
				if (pagina != NULL)
					printf("|%d-%d-%d-%d-%d|", i, a_Memoria[i].pid,
							a_Memoria[i].pag, pagina->bitPuntero,
							pagina->bitUso);
			}

			i++;
		}
		printf("\n");

	}

	if (!strcmp(g_Algoritmo, "LRU")) {
		printf("|LISTA LRU|\n");
		i = 0;
		while (i < list_size(lista_lru)) {
			lru = list_get(lista_lru, i);

			printf("|%d-%d| ", lru->pid, lru->pagina);

			i++;
		}
		printf("\n");
	}

}

int marcosUsados(int pid) {
	int i = 0, contador = 0;

	while (i < g_Cantidad_Marcos) {
		if (a_Memoria[i].pid == pid) {
			contador++;
		}
		i++;
	}

	return contador;
}

void asignarMarco(int pid, int pag, int *marco, int mOcup) {

	a_Memoria[*marco].marcoEnUso = 1;
	a_Memoria[*marco].pid = pid;
	a_Memoria[*marco].pag = pag;
	t_mProc *mProc;
	t_pagina *pagina;
	int i = 0;

	while (i < list_size(lista_mProc)) {
		mProc = list_get(lista_mProc, i);
		if (mProc->pid == pid) {

			pagina = malloc(sizeof(t_pagina));
			pagina->pagina = pag;
			pagina->bitMP = 1;
			pagina->bitModificado = 1;
			pagina->bitUso = 1;
			pagina->marco = *marco;
			pagina->bitPuntero = 0;
			if (mOcup == 0)
				pagina->bitPuntero = 1;

			list_add(mProc->paginas, pagina);

			break;

		}
		i++;
	}

}

int damePaginaVictima(int pid, int* marco) {
	t_mProc *mProc;
	t_pagina *pagina;
	int i = 0, j = 0, cantPag, encontrado = 0, victima;
	int bandera = 0;

	while (i < list_size(lista_mProc)) {
		mProc = list_get(lista_mProc, i);
		if (mProc->pid == pid) {
			cantPag = list_size(mProc->paginas);

			j = damePuntero(mProc->paginas);
			if (j > -1) {
				while (j < cantPag) {
					pagina = list_get(mProc->paginas, j);
					if (pagina->bitUso == 1) {
						pagina->bitUso = 0;
					} else {
						encontrado = 1;
						victima = pagina->pagina;
						*marco = pagina->marco;
					}

					if (encontrado) {
						pagina->bitPuntero = 0;
						j++;
						if (j < cantPag) {
							pagina = list_get(mProc->paginas, j);
							pagina->bitPuntero = 1;

						} else {
							pagina = list_get(mProc->paginas, 0);
							pagina->bitPuntero = 1;

						}
						return victima;
					}

					j++;
					if (j == cantPag && bandera < 2) {
						j = 0;
						bandera++;
						;
					}
				}
			} else {
				printf("NO HAY PUNTERO MAL:%d\n", pid);
				abort();
			}
		}
		i++;
	}

	return -1;

}

int recorrerYDarmeMarco(int pid) {
	int pagina, marco = -1;
	if (!strcmp(g_Algoritmo, "CLOCK")) {
		pagina = damePaginaVictima(pid, &marco);
		printf("PAGINA VICTIMA: %d ---- MARCO VICTIMA: %d\n", pagina, marco);
		grabarContenidoASwap(pid, pagina, a_Memoria[marco].contenido);
		actualizarTablaPagina(pid, pagina);
		memset(a_Memoria[marco].contenido, 0, g_Tamanio_Marco);
	}
	if (!strcmp(g_Algoritmo, "CLOCKMEJORADO")) {
		pagina = damePaginaVictima(pid, &marco);
		printf("PAGINA VICTIMA: %d ---- MARCO VICTIMA: %d\n", pagina, marco);
		grabarContenidoASwap(pid, pagina, a_Memoria[marco].contenido);
		actualizarTablaPagina(pid, pagina);
		memset(a_Memoria[marco].contenido, 0, g_Tamanio_Marco);
	}

	return marco;
}

void poneElBitUso(int pid, int pag) {
	t_mProc *mProc;
	t_pagina *pagina;
	int i = 0, j = 0, cantPag;

	while (i < list_size(lista_mProc)) {
		mProc = list_get(lista_mProc, i);
		if (mProc->pid == pid) {
			cantPag = list_size(mProc->paginas);
			j = 0;
			while (j < cantPag) {
				pagina = list_get(mProc->paginas, j);
				if (pagina->pagina == pag) {
					pagina->bitUso = 1;
					j = cantPag;
				}
				j++;
			}
			break;
		}
		i++;
	}
}

int CLOCK(int *marco, int pagina, int pid) {
	int mOcup = -1;

	*marco = dameMarco();
	if (*marco != -1) {
		mOcup = marcosUsados(pid);
		if (mOcup < g_Maximo_Marcos_Por_Proceso) {
			asignarMarco(pid, pagina, marco, mOcup);
		} else {
			*marco = recorrerYDarmeMarco(pid);
			asignarMarco(pid, pagina, marco, mOcup);
		}
	} else {
		*marco = recorrerYDarmeMarco(pid);

		asignarMarco(pid, pagina, marco, mOcup);

	}

	return 1;
}

void hayLugarEnMPSinoLoHago(int* marco, int pid, int nroPagina, char *contenido) {



	//char* contenido;
	if (!strcmp(g_Algoritmo, "FIFO")) {
		FIFO2(marco, pid, nroPagina);

	} else if (!strcmp(g_Algoritmo, "LRU")) {
		LRU(marco, pid, nroPagina, contenido);

	} else if (!strcmp(g_Algoritmo, "CLOCK")) {
		CLOCK(marco, nroPagina, pid);
	} else if (!strcmp(g_Algoritmo, "CLOCKMEJORADO")) {
		CLOCK(marco, nroPagina, pid);
	}

	if(*marco>=0)
		cantFallos++;
}

int cuentaDigitos(int valor) {
	int cont = 0;
	float tamDigArch = valor;
	while (tamDigArch >= 1) {
		tamDigArch = tamDigArch / 10;
		cont++;
	}
	return cont;
}

char* obtenerSubBufferDeContenido(char *nombre, int tamanio) {

	//Le tengo q pasar un string y un tamanio,
	//El nombre debe tener el resto del contenido con \0, Ej: nombre="hola\0\0\0....\0" la cantidad de \0 son (tamanio-strlen(hola)) PD: Puse el strlen como ejemplo pero no se puede usar xDXDXDXDxXxXdXXdXDXd
	//Ej: nombre= AhiEstaElYetaDeCici\0,\0aTocarMaderaTodos  tamanio=256  ==> salida= 3256AhiEstaElYetaDeCici\0,\0aTocarMaderaTodos\0\0\0\0...\0

	float tam = tamanio;
	int cont = 0;

	char *aux;

	while (tam >= 1) {
		tam = tam / 10;
		cont++;
	}

	aux = malloc(tamanio + cuentaDigitos(cont) + cuentaDigitos(tamanio));
	memset(aux, 0, tamanio + cuentaDigitos(cont) + cuentaDigitos(tamanio));

	memcpy(aux, string_itoa(cont), strlen(string_itoa(cont))); // Son nros, puedo hacer stlren
	memcpy(aux + strlen(string_itoa(cont)), string_itoa(tamanio),
			strlen(string_itoa(tamanio)));   // Son nros, puedo hacer stlren
	memcpy(aux + strlen(string_itoa(cont)) + strlen(string_itoa(tamanio)),
			nombre, tamanio);

	return aux;
}

void implementoEscribirCpu(int socket, char *buffer) {

	//2 3 111 112 14hola

	int posActual = 2, pid, nroPagina = -1, marco = -1;
	char *bufferAux, *contenido;
	contenido = malloc(g_Tamanio_Marco);
	memset(contenido, 0, g_Tamanio_Marco);

	//Id Proceso
	bufferAux = DigitosNombreArchivo(buffer, &posActual);
	pid = atoi(bufferAux);
	free(bufferAux);

	//Numero de Pagina
	bufferAux = DigitosNombreArchivo(buffer, &posActual);
	nroPagina = atoi(bufferAux);
	int pos = posActual, valido = 0;
	t_lru *lru;

	int cantDig = ChartToInt(buffer[pos]);

	pos++;

	int p, tamanioC = 0, aux;
	for (p = 1; p <= cantDig; p++) {
		aux = ChartToInt(buffer[pos++]);
		tamanioC = tamanioC * 10 + aux;
	}

	if (tamanioC <= g_Tamanio_Marco) {

		//Contenido a grabar en la Pagina
		bufferAux = DigitosNombreArchivo(buffer, &posActual);
		memcpy(contenido, bufferAux, tamanioC);

		printf(
				"***********************"NEGRITA"ESCRIBIR"DEFAULT"**********************************\n");
		printf(
				"* CPU solicita escribir Pid:"COLOR_VERDE""NEGRITA"%d"DEFAULT" Pagina:"COLOR_VERDE""NEGRITA"%d"DEFAULT" Contenido:",
				pid, nroPagina);
		imprimirContenido(contenido, tamanioC);
		printf("\n");
		printf("* ("COLOR_VERDE""NEGRITA"Escribir"DEFAULT") ");
		if (g_Cantidad_Marcos > 0) {
			if (buscarPaginaEnTLB(pid, nroPagina, &marco)) {
				//Acierto de la TLB entonces quiere decir que si esta en la TLB esta si o si en la memoria princial

				if(!strcmp(g_Algoritmo,"LRU")){
					lru = malloc(sizeof(t_lru));
					lru->pagina = nroPagina;
					lru->pid = pid;
					list_add(lista_lru, lru);
				}
			} else {

				//imprimirTablaDePaginas();

				if (buscarEnTablaDePaginas(pid, nroPagina, &marco)) {
					if(!strcmp(g_Algoritmo,"LRU")){
						lru = malloc(sizeof(t_lru));
						lru->pagina = nroPagina;
						lru->pid = pid;
						list_add(lista_lru, lru);
					}	//Encontro la pagina en la tabla de paginas
				} else {
					//No encontro la pagina en la Tabla, entonces graba el contenido en la memoria principal si no hay
					// hacemos boleta a alguien
					hayLugarEnMPSinoLoHago(&marco, pid, nroPagina, contenido);


					if (marco == -1) {
						printf(
								"* ("COLOR_VERDE""NEGRITA"Escribir"DEFAULT") Memoria llena\n");
						EnviarDatos(socket, "0", strlen("0"));
						return;
					}

				}
				//sleep(g_Retardo_Memoria);
			}

			actualizarMemoriaPrincipal(pid, nroPagina, contenido, tamanioC,
					marco);
			actualizarTLB(pid, nroPagina);
			imprimirTLB();
			printf("* "NEGRITA"Cantidad Fallos de Pagina "NEGRITA""COLOR_VERDE"%d"DEFAULT"\n",cantFallos);

			printf("* ("COLOR_VERDE""NEGRITA"Escribir"DEFAULT") Contenido:");
			imprimirContenido(a_Memoria[marco].contenido, g_Tamanio_Marco);
			printf("\n");
			printf("* ("COLOR_VERDE""NEGRITA"Escribir"DEFAULT") Marco:%d\n",
					marco);
			EnviarDatos(socket, a_Memoria[marco].contenido, g_Tamanio_Marco);
			//enviarContenidoACpu(socket,pid,nroPagina,a_Memoria[marco].contenido,tamanioC);
		} else {
			valido = grabarContenidoASwap(pid, nroPagina, contenido);
			if (valido) {
				printf("* Se escribio en SWAP-> Pid:%d Pagina:%d Contenido:",
						pid, nroPagina);
				imprimirContenido(contenido, g_Tamanio_Marco);
				printf("\n");
				EnviarDatos(socket, contenido, g_Tamanio_Marco);
			} else {
				printf(
						"No se pudo escribir en Swap -> Pid:%d Pagina:%d Contenido:%s\n",
						pid, nroPagina, contenido);
				EnviarDatos(socket, "0", strlen("0"));
			}
		}
	} else {
		printf(
				"***********************ESCRIBIR-ERROR****************************\n");
		printf(
				"CPU solicita escribir pero envia algo mas grande que el tamaño de marco\n");
		printf("Tamaño de Marco:%d Tamaño que envia CPU:%d\n", g_Tamanio_Marco,
				tamanioC);
		EnviarDatos(socket, "0", strlen("0"));
	}


	imprimirMemoria();


}

void imprimirTLB() {
	int i = 0;
	t_tlb* tlb;
	pthread_mutex_lock(&semTELEBE);
	if (!strcmp(g_TLB_Habilitada, "SI")) {
		printf("***************"NEGRITA"TLB"DEFAULT"***************\n");
		printf(
				"* "COLOR_VERDE""NEGRITA"Pos"DEFAULT"\t"COLOR_VERDE""NEGRITA"Pid"DEFAULT"\t"COLOR_VERDE""NEGRITA"Pag"DEFAULT"\t"COLOR_VERDE""NEGRITA"Marco"DEFAULT"\t*\n");
		printf("*********************************\n");
		while (i < list_size(lista_tlb)) {
			tlb = list_get(lista_tlb, i);
			printf("*  %d\t%d\t%d\t%d\t*\n", i, tlb->pid, tlb->pagina,
					tlb->marco);
			i++;
		}
		printf("*********************************\n");
	}
	pthread_mutex_unlock(&semTELEBE);
}

void implementoLeerCpu(int socket, char *buffer) {

	//2 2 111 112

	int posActual = 2, pid, nroPagina = -1, marco = -1;
	char *bufferAux, *contenido;
	t_lru *lru;

	//Id Proceso
	bufferAux = DigitosNombreArchivo(buffer, &posActual);
	pid = atoi(bufferAux);
	free(bufferAux);

	//Numero de Pagina
	bufferAux = DigitosNombreArchivo(buffer, &posActual);
	nroPagina = atoi(bufferAux);

	printf(
			"**************************"NEGRITA"LEER"DEFAULT"***********************************\n");
	printf(
			"* CPU Solicita leer Pid:"COLOR_VERDE" "NEGRITA"%d"DEFAULT" Pagina:"COLOR_VERDE" "NEGRITA"%d"DEFAULT"\n",
			pid, nroPagina);
	//printf("* ("COLOR_VERDE"Leer"DEFAULT") ");
	if (g_Cantidad_Marcos > 0) {
		if (buscarPaginaEnTLB(pid, nroPagina, &marco)) {
			if(!strcmp(g_Algoritmo,"LRU")){
				lru = malloc(sizeof(t_lru));
				lru->pagina = nroPagina;
				lru->pid = pid;
				list_add(lista_lru, lru);
			}
			//Acierto de la TLB entonces quiere decir que si esta en la TLB esta si o si en la memoria princial
			contenido = buscarEnMemoriaPrincipal(marco);
		} else {

			//imprimirTablaDePaginas();

			if (buscarEnTablaDePaginas(pid, nroPagina, &marco)) {
				//Encontro la pagina en la tabla de paginas
				if(!strcmp(g_Algoritmo,"LRU")){
					lru = malloc(sizeof(t_lru));
					lru->pagina = nroPagina;
					lru->pid = pid;
					list_add(lista_lru, lru);
				}

				contenido = buscarEnMemoriaPrincipal(marco);

			} else {
				//No encontro la pagina en la Tabla, entonces debe pedirla al Swap (si o si va a devolver el contenido el Swap)
				contenido = pedirContenidoASwap(pid, nroPagina);
				if (contenido != NULL) {
					hayLugarEnMPSinoLoHago(&marco, pid, nroPagina, contenido);


					if (marco == -1) {
						printf(
								"* ("COLOR_VERDE""NEGRITA"Leer"DEFAULT") Memoria llena\n");
						EnviarDatos(socket, "0", strlen("0"));
						return;
					}

					actualizarMemoriaPrincipal(pid, nroPagina, contenido,
							g_Tamanio_Marco, marco);
				} else {
					printf(
							"* ("COLOR_VERDE""NEGRITA"Leer"DEFAULT") PUCHA!! SWAP NO PUDO LEER LA PAGINA\n");
				}
			}
			if (contenido != NULL) {
				actualizarTLB(pid, nroPagina);
				//	imprimirTLB();
			}

			//sleep(g_Retardo_Memoria);
		}
		imprimirTLB();
		printf("* "NEGRITA"Cantidad Fallos de Pagina "NEGRITA""COLOR_VERDE"%d"DEFAULT"\n",cantFallos);
		printf(
				"* ("COLOR_VERDE""NEGRITA"Leer"DEFAULT") Busco en MP. Contenido:");
		imprimirContenido(a_Memoria[marco].contenido, g_Tamanio_Marco);
		printf("\n");

		if (contenido != NULL) {
			EnviarDatos(socket, a_Memoria[marco].contenido, g_Tamanio_Marco);
		} else {
			EnviarDatos(socket, "0", strlen("0"));
		}
	} else {
		contenido = pedirContenidoASwap(pid, nroPagina);
		if (contenido != NULL) {
			printf(
					"* ("COLOR_VERDE""NEGRITA"Leer"DEFAULT") Busco en SWAP Contenido:");
			imprimirContenido(contenido, g_Tamanio_Marco);
			printf("\n");
			EnviarDatos(socket, contenido, g_Tamanio_Marco);
		} else {
			EnviarDatos(socket, "0", strlen("0"));
		}
	}

	imprimirMemoria();

	//enviarContenidoACpu(socket,pid,nroPagina,a_Memoria[marco].contenido,g_Tamanio_Marco);
}

int eliminarDeSwap(int pid) {

	char* bufferASwap = string_new();
	char* bufferRespuesta = string_new();

	string_append(&bufferASwap, "34");
	string_append(&bufferASwap, obtenerSubBuffer(string_itoa(pid)));

	pthread_mutex_lock(&semSwap);
	EnviarDatos(socket_Swap, bufferASwap, strlen(bufferASwap));

	RecibirDatos(socket_Swap, &bufferRespuesta);
	pthread_mutex_unlock(&semSwap);

	printf(
			"* ("COLOR_VERDE""NEGRITA"Finalizar"DEFAULT") Respuesta de SWAP: %s\n",
			bufferRespuesta);

	if (strcmp(bufferRespuesta, "1") == 0) {

		//Swap pudo eliminar to do sobre ese proceso
		return 1;

	} else {
		//Swap se puso la gorra y no elimino nada

		return 0;

	}

}

int eliminarProceso(int pid) {

	t_mProc *mProc;
	t_pagina *pagina;
	int i = 0;

	while (i < list_size(lista_mProc)) {
		mProc = list_get(lista_mProc, i);

		if (mProc->pid == pid) {

			while (list_size(mProc->paginas) > 0) {

				pagina = list_remove(mProc->paginas, 0);

				if (pagina->bitMP == 1) {

					a_Memoria[pagina->marco].marcoEnUso = 0;
					a_Memoria[pagina->marco].pid = -1;
					a_Memoria[pagina->marco].pag = -1;
					memset(a_Memoria[pagina->marco].contenido, 0,
							g_Tamanio_Marco);

				}

			}

			list_remove(lista_mProc, i);

			if (eliminarDeSwap(pid)) {

				//Se elimino con exito

				return 1;

			} else {

				//No se elimino un carajo

				return 0;

			}

		}
		i++;
	}

	return 0;

}

void eliminarDeLaTlbEnFinalizar(int pid) {

	t_tlb *tlb;
	int i = 0;
	pthread_mutex_lock(&semTELEBE);

	while (i < list_size(lista_tlb)) {

		tlb = list_get(lista_tlb, i);

		if (tlb->pid == pid) {

			tlb->marco = -1;
			tlb->pagina = -1;
			tlb->pid = -1;
		}
		i++;
	}

	pthread_mutex_unlock(&semTELEBE);

}

void eliminarDeListaLRU(int pid) {

	t_lru *lru;
	int i = 0;

	while (i < list_size(lista_lru)) {
		lru = list_get(lista_lru, i);

		if (lru->pid == pid) {
			lru = list_remove(lista_lru, i);
			free(lru);
			i--;
		}

		i++;
	}

}

void implementoFinalizarCpu(int socket, char *buffer) {

	//2 4 111

	int posActual = 2, pid = -1;
	char *bufferAux;
	char *bufferACPU = string_new();

	//Id Proceso
	bufferAux = DigitosNombreArchivo(buffer, &posActual);
	pid = atoi(bufferAux);
	free(bufferAux);

	printf(
			"***************************"NEGRITA"FINALIZAR"DEFAULT"*****************************\n");
	printf("* CPU Solicita finalizar Pid:"COLOR_VERDE""NEGRITA"%d"DEFAULT"\n",
			pid);

	//TODAVIA NO SE SI ACTUALIZAR TLB EN ESTE CASO
	if (g_Entradas_TLB > 0)
		eliminarDeLaTlbEnFinalizar(pid);
	//
	if (!strcmp(g_Algoritmo, "LRU"))
		eliminarDeListaLRU(pid);

	if (eliminarProceso(pid)) {
		//Envio a CPU el OK de q se borro

		string_append(&bufferACPU, "1");

	} else {

		string_append(&bufferACPU, "0");

	}

	EnviarDatos(socket, bufferACPU, strlen(bufferACPU));
	imprimirMemoria();

}

void implementoCPU(int socket, char* buffer) {
	int operacion = ObtenerComandoMSJ(buffer + 1);

	switch (operacion) {
	case INICIAR:
		pthread_mutex_lock(&semMemPrincipal);
		implementoIniciarCpu(socket, buffer);
		pthread_mutex_unlock(&semMemPrincipal);
		//Reservar memoria en Swap
		//Devolver un Ok a CPU
		break;
	case LEER:
		pthread_mutex_lock(&semMemPrincipal);
		implementoLeerCpu(socket, buffer);
		pthread_mutex_unlock(&semMemPrincipal);
		//fijarse si la pagina esta en la TLB
		//si esta, tomar el marco y devolver el contenido a la cpu
		//si no esta, buscarla en la memoria principal y ver si esa pagina esta en swap
		//si no esta en swap devolverle el contenido a la cpu
		//si esta en swap pedirle ese marco al swap, cargarlo en la pagina principal teniendo en cuenta
		//el algoritmo de reemplazo, luego devolverle el contenido a la cpu
		break;
	case ESCRIBIR:
		pthread_mutex_lock(&semMemPrincipal);
		implementoEscribirCpu(socket, buffer);
		pthread_mutex_unlock(&semMemPrincipal);

		//Fijarse si la pagina a escribir esta en la TLB, si esta ir a la memoria principal y reemplazar el marco
		//dejar marcada esa pagina como modificada en la memoria principal
		//si no esta en la tlb, ir a buscarla a la memoria principal y fijarse si ese marco esta en swap o no,
		//si no esta en swap reemplazar su contenido y marcarla como modificada
		//si esta en swap, ir a buscarla, reemplazar algun marco existente teniendo en cuenta el algoritmo de
		//reemplazo y luego reemplazar su contenido
		//decidir si le devolvemos un ok o el contenido grabado a la cpu
		break;
	case FINALIZAR:
		pthread_mutex_lock(&semMemPrincipal);
		implementoFinalizarCpu(socket, buffer);
		pthread_mutex_unlock(&semMemPrincipal);
		//limpiar los marcos reservados de swap del proceso y luego borrar la tabla de paginas y devolver
		// un ok a la cpu
		break;
	default:
		break;
	}
}

int AtiendeCliente(void * arg) {
	int socket = (int) arg;

	// Es el encabezado del mensaje. Nos dice quien envia el mensaje
	int emisor = 0;

	// Dentro del buffer se guarda el mensaje recibido por el cliente.
	char* buffer;
	buffer = malloc(BUFFERSIZE * sizeof(char)); //-> de entrada lo instanciamos en 1 byte, el tamaño será dinamico y dependerá del tamaño del mensaje.

	// Cantidad de bytes recibidos.
	long unsigned bytesRecibidos;

	// La variable fin se usa cuando el cliente quiere cerrar la conexion: chau chau!
	int desconexionCliente = 0;

	// Código de salida por defecto
	int code = 0;
	while ((!desconexionCliente) && g_Ejecutando) {
		if (buffer != NULL)
			free(buffer);
		buffer = string_new();

		//Recibimos los datos del cliente
		bytesRecibidos = RecibirDatos(socket, &buffer);

		if (bytesRecibidos > 0) {
			//Analisamos que peticion nos está haciendo (obtenemos el comando)
			emisor = ObtenerComandoMSJ(buffer);

			//Evaluamos los comandos
			switch (emisor) {
			case ES_CPU:

				implementoCPU(socket, buffer);

				break;
			case COMANDO:
				printf("Ejecutado por telnet");
				EnviarDatos(socket, "Ok", strlen("Ok"));
				break;
			default:
				/*procesarBuffer(buffer,bytesRecibidos);
				 enviarArchivo();
				 free(buffer);
				 buffer = string_new();
				 char *  buffer2 = string_new();
				 bytesRecibidos = RecibirDatos(socket_Swap,&buffer2);
				 procesarBuffer2(buffer2,bytesRecibidos);
				 enviarArchivo2(socket);
				 free(buffer2);*/
				break;
			}
		} else
			desconexionCliente = 1;
	}
	CerrarSocket(socket);
	return code;
}

int conectarSWAP() {
	sem_wait(&semLog);
	log_info(logger, "Intentando conectar a Swap\n");
	sem_post(&semLog);

	struct addrinfo hints;
	struct addrinfo *serverInfo;
	int conexionOk = 0;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	if (getaddrinfo(g_Ip_Swap, g_Puerto_Swap, &hints, &serverInfo) != 0) {// Carga en serverInfo los datos de la conexion
		sem_wait(&semLog);
		log_info(logger, "ERROR: cargando datos de conexion socket_FS");
		sem_post(&semLog);
	}

	if ((socket_Swap = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol)) < 0) {
		sem_wait(&semLog);
		log_info(logger, "ERROR: crear socket_FS");
		sem_post(&semLog);
	}
	if (connect(socket_Swap, serverInfo->ai_addr, serverInfo->ai_addrlen) < 0) {
		sem_wait(&semLog);
		log_info(logger, "ERROR: conectar socket_FS");
		sem_post(&semLog);
	} else {
		conexionOk = 1;
	}
	freeaddrinfo(serverInfo);	// No lo necesitamos mas
	return conexionOk;
}

int conexionASwap() {
	//int cont;
	//int bytesRecibidos,cantRafaga=1,tamanio;
	//char*buffer = string_new();
	//char*bufferR = string_new();
	//char*bufferE = string_new();
	//char*aux;

	if (conectarSWAP()) {
		printf("Se conecto a SWAP\n");
		//ENVIO a FILESYSTEM(la segunda rafaga)
		//31210127.0.0.1143000
		//string_append(&buffer,"31");
		//aux=obtenerSubBuffer(g_Ip_Nodo);
		//string_append(&buffer,aux);
		//aux=obtenerSubBuffer(string_itoa(g_Puerto_Nodo));
		//string_append(&buffer,aux);
		//aux=obtenerSubBuffer(LuToChar(tamanio_archivo(g_Archivo_Bin)));
		//string_append(&buffer,aux);

		//string_append(&bufferE,"3");
		//cont = cuentaDigitos(strlen(buffer));
		//string_append(&bufferE,string_itoa(cont));
		//string_append(&bufferE,string_itoa(strlen(buffer)));

		//Primera Rafaga
		//sem_wait(&semCon);
		//EnviarDatos(socket_Fs,bufferE, strlen(bufferE));
		//sem_post(&semCon);
		//bufferR = RecibirDatos(socket_Fs,bufferR, &bytesRecibidos,&cantRafaga,&tamanio);
		//Recibo respuesta de FS
		//if(bufferR!=NULL){
		//Segunda Rafaga
		//sem_wait(&semCon);
		//EnviarDatos(socket_Fs,buffer, strlen(buffer));
		//sem_post(&semCon);
		//bufferR = RecibirDatos(socket_Fs,bufferR, &bytesRecibidos,&cantRafaga,&tamanio);
		//printf("\nNodo Conectado al Fs!");
		//} else {
		//printf("No se pudo conectar al FS\n");
		//}
		//free(buffer);
		//free(bufferR);
		//free(bufferE);
		return 1;
	}
	return 0;
}

void CerrarSocket(int socket) {
	close(socket);
	sem_wait(&semLog);
	log_trace(logger, "SOCKET SE CIERRA: (%d).", socket);
	sem_post(&semLog);
}

void ErrorFatal(const char* mensaje, ...) {
	char* nuevo;
	va_list arguments;
	va_start(arguments, mensaje);
	nuevo = string_from_vformat(mensaje, arguments);
	printf("\nERROR FATAL--> %s \n", nuevo);
	sem_wait(&semLog);

	log_error(logger, "\nERROR FATAL--> %s \n", nuevo);
	sem_post(&semLog);
	char fin;

	printf(
			"El programa se cerrara. Presione ENTER para finalizar la ejecución.");
	fin = scanf("%c", &fin);

	va_end(arguments);
	if (nuevo != NULL)
		free(nuevo);
	exit(EXIT_FAILURE);
}

int ObtenerComandoMSJ(char* buffer) {
//Hay que obtener el comando dado el buffer.
//El comando está dado por el primer caracter, que tiene que ser un número.
	return PosicionDeBufferAInt(buffer, 0);
}

int PosicionDeBufferAInt(char* buffer, int posicion) {
	int logitudBuffer = 0;
	logitudBuffer = strlen(buffer);

	if (logitudBuffer <= posicion)
		return 0;
	else
		return ChartToInt(buffer[posicion]);
}

int ChartToInt(char x) {
	int numero = 0;
	char * aux = string_new();
	string_append_with_format(&aux, "%c", x);
	//char* aux = malloc(1 * sizeof(char));
	//sprintf(aux, "%c", x);
	numero = strtol(aux, (char **) NULL, 10);

	if (aux != NULL)
		free(aux);
	return numero;
}

long unsigned RecibirDatos(int socket, char **buffer) {
	long bytesRecibidos = 0, tamanioBuffer = 0, bytesEnviados;
	char *bufferAux = malloc(1);
	int posicion = 1;
	memset(bufferAux, 0, 1);

	bufferAux = realloc(bufferAux, BUFFERSIZE * sizeof(char) + 1);

	memset(bufferAux, 0, BUFFERSIZE * sizeof(char) + 1); //-> llenamos el bufferAux con barras ceros.

	if ((bytesRecibidos = bytesRecibidos
			+ recv(socket, bufferAux, BUFFERSIZE, 0)) == -1) {
		Error(
				"Ocurrio un error al intentar recibir datos desde uno de los clientes. Socket: %d",
				socket);
	}

	if (bytesRecibidos > 0) {
		bytesEnviados = send(socket, "Ok", strlen("Ok"), 0);
		if (bytesEnviados <= 0)
			return 0;
	} else
		return 0;

	tamanioBuffer = atoi(DigitosNombreArchivo(bufferAux, &posicion));

	free(bufferAux);

	bufferAux = malloc(tamanioBuffer + 1);
	*buffer = malloc(tamanioBuffer + 10);
	memset(bufferAux, 0, tamanioBuffer + 1);
	memset(*buffer, 0, tamanioBuffer + 10);

	ssize_t numBytesRecv = 0;

	long unsigned pos = 0;
	do {
		numBytesRecv = recv(socket, bufferAux, tamanioBuffer, 0);
		if (numBytesRecv < 0)
			printf("ERROR\n");
		//printf("Recibido:%lu\n",pos);
		memcpy((*buffer + pos), bufferAux, numBytesRecv);
		memset(bufferAux, 0, tamanioBuffer + 1);
		pos = pos + numBytesRecv;
	} while (pos < tamanioBuffer);

	sem_wait(&semLog);
	log_trace(logger, "RECIBO DATOS. socket: %d. tamanio buffer:%lu", socket,
			tamanioBuffer);
	sem_post(&semLog);
	return tamanioBuffer;
}

char* DigitosNombreArchivo(char *buffer, int *posicion) {

	char *nombreArch;
	int digito = 0, i = 0, j = 0, algo = 0, aux = 0, x = 0;

	digito = PosicionDeBufferAInt(buffer, *posicion);
	for (i = 1; i <= digito; i++) {
		algo = PosicionDeBufferAInt(buffer, *posicion + i);
		aux = aux * 10 + algo;
	}
	nombreArch = malloc(aux + 1);
	for (j = *posicion + i; j < *posicion + i + aux; j++) {
		nombreArch[x] = buffer[j];
		x++;
	}
	nombreArch[x] = '\0';
	*posicion = *posicion + i + aux;
	return nombreArch;
}

long unsigned EnviarDatos(int socket, char *buffer, long unsigned tamanioBuffer) {

	//printf("ENVIO DATOS:%s\n",buffer);

	int bytecount, bytesRecibidos;
	long unsigned cantEnviados = 0;
	char * bufferE = string_new(), *bufferR = malloc(BUFFERSIZE);
	memset(bufferR, 0, BUFFERSIZE);

	string_append(&bufferE, YO);

	string_append(&bufferE, obtenerSubBuffer(string_itoa(tamanioBuffer)));

	if ((bytecount = send(socket, bufferE, strlen(bufferE), 0)) == -1) {
		Error("No puedo enviar información a al cliente. Socket: %d", socket);
		return 0;
	}

	if ((bytesRecibidos = recv(socket, bufferR, BUFFERSIZE, 0)) == -1) {
		Error(
				"Ocurrio un error al intentar recibir datos desde uno de los clientes. Socket: %d",
				socket);
	}

	long unsigned n, bytesleft = tamanioBuffer;

	while (cantEnviados < tamanioBuffer) {
		n = send(socket, buffer + cantEnviados, bytesleft, 0);
		if (n == -1) {
			Error("Fallo al enviar a Socket: %d,", socket);
			return 0;
		}
		cantEnviados += n;
		bytesleft -= n;
		//printf("Cantidad Enviada :%lu\n",n);
	}
	if (cantEnviados != tamanioBuffer)
		return 0;
	sem_wait(&semLog);
	log_info(logger, "ENVIO DATOS. socket: %d. Cantidad Enviada:%lu ", socket,
			tamanioBuffer);
	sem_post(&semLog);
	return tamanioBuffer;
}

#if 1 // METODOS CONFIGURACION //
void LevantarConfig() {
	t_config* config = config_create(PATH_CONFIG);
	// Nos fijamos si el archivo de conf. pudo ser leido y si tiene los parametros

	if (config->properties->table_current_size != 0) {

		// Puerto de escucha
		if (config_has_property(config, "PUERTO_ESCUCHA")) {
			g_Puerto = config_get_int_value(config, "PUERTO_ESCUCHA");
		} else
			Error("No se pudo leer el parametro PUERTO_ESCUCHA");
		if (config_has_property(config, "IP_SWAP")) {
			g_Ip_Swap = config_get_string_value(config, "IP_SWAP");
		} else
			Error("No se pudo leer el parametro IP_SWAP");
		if (config_has_property(config, "PUERTO_SWAP")) {
			g_Puerto_Swap = config_get_string_value(config, "PUERTO_SWAP");
		} else
			Error("No se pudo leer el parametro PUERTO_SWAP");
		if (config_has_property(config, "MAXIMO_MARCOS_POR_PROCESO")) {
			g_Maximo_Marcos_Por_Proceso = config_get_int_value(config,
					"MAXIMO_MARCOS_POR_PROCESO");
		} else
			Error("No se pudo leer el parametro MAXIMO_MARCOS_POR_PROCESO");
		if (config_has_property(config, "CANTIDAD_MARCOS")) {
			g_Cantidad_Marcos = config_get_int_value(config, "CANTIDAD_MARCOS");
		} else
			Error("No se pudo leer el parametro CANTIDAD_MARCOS");
		if (config_has_property(config, "TAMANIO_MARCO")) {
			g_Tamanio_Marco = config_get_int_value(config, "TAMANIO_MARCO");
		} else
			Error("No se pudo leer el parametro TAMANIO_MARCO");
		if (config_has_property(config, "ENTRADAS_TLB")) {
			g_Entradas_TLB = config_get_int_value(config, "ENTRADAS_TLB");
		} else
			Error("No se pudo leer el parametro ENTRADAS_TLB");
		if (config_has_property(config, "TLB_HABILITADA")) {
			g_TLB_Habilitada = config_get_string_value(config,
					"TLB_HABILITADA");
		} else
			Error("No se pudo leer el parametro TLB_HABILITADA");
		if (config_has_property(config, "RETARDO_MEMORIA")) {
			g_Retardo_Memoria = config_get_int_value(config, "RETARDO_MEMORIA");
		} else
			Error("No se pudo leer el parametro RETARDO_MEMORIA");
		if (config_has_property(config, "ALGORITMO_REEMPLAZO")) {
			g_Algoritmo = config_get_string_value(config,
					"ALGORITMO_REEMPLAZO");
		} else
			Error("No se pudo leer el parametro RETARDO_MEMORIA");
	} else {
		ErrorFatal("No se pudo abrir el archivo de configuracion");
	}
	if (config != NULL) {
		free(config);
	}
}

#endif

#if 1 // METODOS MANEJO DE ERRORES //
void Error(const char* mensaje, ...) {
	char* nuevo;
	va_list arguments;
	va_start(arguments, mensaje);
	nuevo = string_from_vformat(mensaje, arguments);

	fprintf(stderr, "\nERROR: %s\n", nuevo);
	sem_wait(&semLog);
	log_error(logger, "%s", nuevo);
	sem_post(&semLog);
	va_end(arguments);
	if (nuevo != NULL)
		free(nuevo);
}
#endif

char* obtenerSubBuffer(char *nombre) {
	// Esta funcion recibe un nombre y devuelve ese nombre de acuerdo al protocolo. Ej: carlos ------> 16carlos
	char *aux = string_new();
	int tamanioNombre = 0;
	float tam = 0;
	int cont = 0;

	tamanioNombre = strlen(nombre);
	tam = tamanioNombre;
	while (tam >= 1) {
		tam = tam / 10;
		cont++;
	}
	string_append(&aux, string_itoa(cont));
	string_append(&aux, string_itoa(tamanioNombre));
	string_append(&aux, nombre);

	return aux;
}

void imprimirContenido(char* contenido, long unsigned tamanio) {
	long unsigned i = 0;
	while (i < tamanio) {
		if (*(contenido + i) != '\0') {
			printf("%c", *(contenido + i));
		} else {
			printf("\\0");
		}
		i++;
	}
}
