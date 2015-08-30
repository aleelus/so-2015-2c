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
	logger = log_create(NOMBRE_ARCHIVO_LOG, "cpu", false, LOG_LEVEL_TRACE);

	// Levantamos el archivo de configuracion.
	LevantarConfig();

	iniciarTLB();

	iniciarListamProc();

	//iniciarMemoriaPrincipal();

	//Me Conecto a Swap
	if(conexionASwap()){
		//Hilo orquestador conexiones
		int iThreadOrquestador = pthread_create(&hOrquestadorConexiones, NULL,
				(void*) HiloOrquestadorDeConexiones, NULL );
		if (iThreadOrquestador) {
			fprintf(stderr,
				"Error al crear hilo - pthread_create() return code: %d\n",
				iThreadOrquestador);
			exit(EXIT_FAILURE);
		}
		pthread_join(hOrquestadorConexiones, NULL );
	} else {
		printf("Error, no se pudo conectar a SWAP.\n");
		return -1;
	}
	return 0;
}

void iniciarListamProc(){
	lista_mProc = list_create();
}

void iniciarTLB(){
	if(!strcmp(g_TLB_Habilitada,"SI")){
		lista_tlb = list_create();
		printf("TLB Habilitada - Entradas:%d\n",g_Entradas_TLB);
		int i;
		for(i=0;i<g_Entradas_TLB;i++){
			t_tlb* tlb = malloc(sizeof(t_tlb));
			tlb->marco = -1;
			tlb->pagina = -1;
			list_add(lista_tlb,tlb);
		}
	} else {
		printf("TLB No Habilitada\n");
	}
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
	my_addr.sin_addr.s_addr = htons(INADDR_ANY );
	memset(&(my_addr.sin_zero), '\0', 8 * sizeof(char));

	if (bind(socket_host, (struct sockaddr*) &my_addr, sizeof(my_addr)) == -1)
		ErrorFatal("Error al hacer el Bind. El puerto está en uso");

	if (listen(socket_host, 10) == -1) // el "10" es el tamaño de la cola de conexiones.
		ErrorFatal(
				"Error al hacer el Listen. No se pudo escuchar en el puerto especificado");

	//Traza("El socket está listo para recibir conexiones. Numero de socket: %d, puerto: %d", socket_host, g_Puerto);
	log_trace(logger,
			"SOCKET LISTO PARA RECIBIR CONEXIONES. Numero de socket: %d, puerto: %d",
			socket_host, g_Puerto);

	while (g_Ejecutando) {
		int socket_client;

		size_addr = sizeof(struct sockaddr_in);

		if ((socket_client = accept(socket_host,(struct sockaddr *) &client_addr, &size_addr)) != -1) {
			//Traza("Se ha conectado el cliente (%s) por el puerto (%d). El número de socket del cliente es: %d", inet_ntoa(client_addr.sin_addr), client_addr.sin_port, socket_client);
			log_trace(logger,
					"NUEVA CONEXION ENTRANTE. Se ha conectado el cliente (%s) por el puerto (%d). El número de socket del cliente es: %d",
					inet_ntoa(client_addr.sin_addr), client_addr.sin_port,
					socket_client);
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

void procesarBuffer(char* buffer, long unsigned tamanioBuffer){

	FILE * archivo = fopen("texto.txt","w");

	fwrite(buffer,1,tamanioBuffer,archivo);

	fclose(archivo);

}

void procesarBuffer2(char* buffer, long unsigned tamanioBuffer){

	FILE * archivo = fopen("texto2.txt","w");

	fwrite(buffer,1,tamanioBuffer,archivo);

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

	EnviarDatos(socket_Swap,contenido,tamanioA);

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

	EnviarDatos(socket,contenido,tamanioA);

	free(contenido);

}

void implementoCPU(char* buffer,t_mProc* mProc){
	int operacion = ObtenerComandoMSJ(buffer+1);
	switch (operacion) {
	case INICIAR:
		//Reservar memoria en Swap
		//Devolver un Ok a CPU
		break;
	case LEER:
		//fijarse si la pagina esta en la TLB
		//si esta, tomar el marco y devolver el contenido a la cpu
		//si no esta, buscarla en la memoria principal y ver si esa pagina esta en swap
		//si no esta en swap devolverle el contenido a la cpu
		//si esta en swap pedirle ese marco al swap, cargarlo en la pagina principal teniendo en cuenta
		//el algoritmo de reemplazo, luego devolverle el contenido a la cpu
		break;
	case ESCRIBIR:
		//Fijarse si la pagina a escribir esta en la TLB, si esta ir a la memoria principal y reemplazar el marco
		//dejar marcada esa pagina como modificada en la memoria principal
		//si no esta en la tlb, ir a buscarla a la memoria principal y fijarse si ese marco esta en swap o no,
		//si no esta en swap reemplazar su contenido y marcarla como modificada
		//si esta en swap, ir a buscarla, reemplazar algun marco existente teniendo en cuenta el algoritmo de
		//reemplazo y luego reemplazar su contenido
		//decidir si le devolvemos un ok o el contenido grabado a la cpu
		break;
	case FINALIZAR:
		//limpiar los marcos reservados de swap del proceso y luego borrar la tabla de paginas y devolver
		// un ok a la cpu
		break;
	default:
		break;
	}
}

int AtiendeCliente(void * arg) {
	int socket = (int) arg;

	//Esctructura mProc
	t_mProc* mProc = malloc(sizeof(t_mProc));

	//Inicia Tabla de Pagina para el Proceso
	mProc->paginas = list_create();

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
		if (buffer != NULL )
			free(buffer);
		buffer = string_new();

		//Recibimos los datos del cliente
		bytesRecibidos = RecibirDatos(socket,&buffer);

		if (bytesRecibidos > 0) {
			//Analisamos que peticion nos está haciendo (obtenemos el comando)
			emisor = ObtenerComandoMSJ(buffer);

			//Evaluamos los comandos
			switch (emisor) {
			case ES_CPU:
				implementoCPU(buffer,mProc);
				break;
			case COMANDO:
				printf("Ejecutado por telnet");
				EnviarDatos(socket,"Ok",strlen("Ok"));
				break;
			default:
				procesarBuffer(buffer,bytesRecibidos);
				enviarArchivo();
				free(buffer);
				buffer = string_new();
				char *  buffer2 = string_new();
				bytesRecibidos = RecibirDatos(socket_Swap,&buffer2);
				procesarBuffer2(buffer2,bytesRecibidos);
				enviarArchivo2(socket);
				free(buffer2);
				break;
			}
		} else
			desconexionCliente = 1;
	}
	CerrarSocket(socket);
	return code;
}


int conectarSWAP() {

	//ESTRUCTURA DE SOCKETS; EN ESTE CASO CONECTA CON NODO
	//log_info(logger, "Intentando conectar a nodo\n");
	//conectar con Nodo
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	int conexionOk = 0;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP


	if (getaddrinfo(g_Ip_Swap, g_Puerto_Swap, &hints, &serverInfo) != 0) {// Carga en serverInfo los datos de la conexion
		log_info(logger,
				"ERROR: cargando datos de conexion socket_FS");
	}

	if ((socket_Swap = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol)) < 0) {
		log_info(logger, "ERROR: crear socket_FS");
	}
	if (connect(socket_Swap, serverInfo->ai_addr, serverInfo->ai_addrlen)
			< 0) {
		log_info(logger, "ERROR: conectar socket_FS");
	} else {
		conexionOk = 1;
	}
	freeaddrinfo(serverInfo);	// No lo necesitamos mas
	return conexionOk;
}
