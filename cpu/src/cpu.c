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

#include "cpu.h"


int main(void) {
	//Si el tercer parametro es true graba en archivo y muestra en pantalla sino solo graba en archivo
	logger = log_create(NOMBRE_ARCHIVO_LOG, "cpu", false, LOG_LEVEL_TRACE);

	sem_init(&semDormilon,0,0);

	// Levantamos el archivo de configuracion.
	LevantarConfig();

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

	EnviarDatos(socket_Memoria,contenido,tamanioA);

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

	EnviarDatos(socket,contenido,tamanioA);

	free(contenido);

}

void ProcesoCPU() {
	int socket_Planificador;

	//log_info(logger, "Intentando conectar a Planificador\n");
	ConectarPlanificador(&socket_Planificador);

	// Es el encabezado del mensaje. Nos dice quien envia el mensaje
	int emisor = 0;

	// Dentro del buffer se guarda el mensaje recibido por el cliente.
	char* buffer;
	buffer = malloc(BUFFERSIZE * sizeof(char)); //-> de entrada lo instanciamos en 1 byte, el tamaño será dinamico y dependerá del tamaño del mensaje.

	// Cantidad de bytes recibidos.
	long unsigned bytesRecibidos;

	// La variable fin se usa cuando el cliente quiere cerrar la conexion: chau chau!
	int desconexionPlanificador = 0;

	// Código de salida por defecto
	while ((!desconexionPlanificador) && g_Ejecutando) {
		if (buffer != NULL )
			free(buffer);
		buffer = string_new();

		//Recibimos los datos del cliente
		bytesRecibidos = RecibirDatos(socket_Planificador,&buffer);

		if (bytesRecibidos > 0) {
			//Analisamos que peticion nos está haciendo (obtenemos el comando)
			emisor = ObtenerComandoMSJ(buffer);

			//Evaluamos los comandos
			switch (emisor) {
			case ES_PLANIFICADOR:
				printf("Hola Planificador\n");
				EnviarDatos(socket_Planificador,"Ok",strlen("Ok"));
				break;
			case COMANDO:
				printf("Ejecutado por telnet");
				EnviarDatos(socket_Planificador,"Ok",strlen("Ok"));
				break;
			default:
				procesarBuffer(buffer,bytesRecibidos);
				enviarArchivo();
				free(buffer);
				buffer=string_new();
				char *  buffer2 = string_new();
				bytesRecibidos = RecibirDatos(socket_Memoria,&buffer2);
				procesarBuffer2(buffer2,bytesRecibidos);
				enviarArchivo2(socket_Planificador);
				free(buffer2);
				break;
			}
		} else
			desconexionPlanificador = 1;
	}
	CerrarSocket(socket_Planificador);
	pthread_exit(NULL);
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

void CerrarSocket(int socket) {
	close(socket);
	//Traza("SOCKET SE CIERRA: (%d).", socket);
	log_trace(logger, "SOCKET SE CIERRA: (%d).", socket);
}

void ErrorFatal(const char* mensaje, ...) {
	char* nuevo;
	va_list arguments;
	va_start(arguments, mensaje);
	nuevo = string_from_vformat(mensaje, arguments);
	printf("\nERROR FATAL--> %s \n", nuevo);
	log_error(logger, "\nERROR FATAL--> %s \n", nuevo);
	char fin;

	printf("El programa se cerrara. Presione ENTER para finalizar la ejecución.");
	fin = scanf("%c", &fin);

	va_end(arguments);
	if (nuevo != NULL )
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

	if (aux != NULL )
		free(aux);
	return numero;
}

long unsigned RecibirDatos(int socket, char **buffer) {
	long bytesRecibidos = 0,tamanioBuffer=0,bytesEnviados;
	char *bufferAux= malloc(1);
	int posicion=1;
	memset(bufferAux,0,1);

	bufferAux = realloc(bufferAux,BUFFERSIZE * sizeof(char)+1);

	memset(bufferAux, 0, BUFFERSIZE * sizeof(char)+1); //-> llenamos el bufferAux con barras ceros.

	if ((bytesRecibidos = bytesRecibidos+recv(socket, bufferAux, BUFFERSIZE, 0)) == -1) {
		Error("Ocurrio un error al intentar recibir datos desde uno de los clientes. Socket: %d",socket);
	}

	if(bytesRecibidos>0) {
		bytesEnviados = send(socket, "Ok", strlen("Ok"), 0);
		if(bytesEnviados<=0) return 0;
	} else return 0;

	tamanioBuffer = atoi(DigitosNombreArchivo(bufferAux,&posicion));

	free(bufferAux);

	bufferAux = malloc(tamanioBuffer+1);
	*buffer = malloc(tamanioBuffer+10);
	memset(bufferAux,0,tamanioBuffer+1);
	memset(*buffer,0,tamanioBuffer+10);

	ssize_t numBytesRecv = 0;

	long unsigned pos=0;
	do{
		numBytesRecv = recv(socket, bufferAux, tamanioBuffer, 0);
		if ( numBytesRecv < 0)
			printf("ERROR\n");
		//printf("Recibido:%lu\n",pos);
		memcpy((*buffer+pos),bufferAux,numBytesRecv);
		memset(bufferAux,0,tamanioBuffer+1);
		pos = pos + numBytesRecv;
	}while (pos < tamanioBuffer);

	log_trace(logger, "RECIBO DATOS. socket: %d. tamanio buffer:%lu", socket,tamanioBuffer);
	return tamanioBuffer;
}
char* DigitosNombreArchivo(char *buffer,int *posicion){

	char *nombreArch;
	int digito=0,i=0,j=0,algo=0,aux=0,x=0;

	digito=PosicionDeBufferAInt(buffer,*posicion);
	for(i=1;i<=digito;i++){
		algo=PosicionDeBufferAInt(buffer,*posicion+i);
		aux=aux*10+algo;
	}
	nombreArch = malloc(aux+1);
	for(j=*posicion+i;j<*posicion+i+aux;j++){
		nombreArch[x]=buffer[j];
		x++;
	}
	nombreArch[x]='\0';
	*posicion=*posicion+i+aux;
	return nombreArch;
}

long unsigned EnviarDatos(int socket, char *buffer, long unsigned tamanioBuffer) {

	int bytecount,bytesRecibidos;
	long unsigned cantEnviados=0;
	char * bufferE = string_new(),*bufferR=malloc(BUFFERSIZE);
	memset(bufferR,0,BUFFERSIZE);

	string_append(&bufferE,YO);

	string_append(&bufferE,obtenerSubBuffer(string_itoa(tamanioBuffer)));

	if ((bytecount = send(socket,bufferE,strlen(bufferE), 0)) == -1){
		Error("No puedo enviar información a al cliente. Socket: %d", socket);
		return 0;
	}

	if ((bytesRecibidos = recv(socket, bufferR, BUFFERSIZE, 0)) == -1) {
		Error("Ocurrio un error al intentar recibir datos desde uno de los clientes. Socket: %d",socket);
	}

	long unsigned n,bytesleft=tamanioBuffer;

	while(cantEnviados < tamanioBuffer) {
		n = send(socket, buffer+cantEnviados, bytesleft, 0);
		if (n == -1){
			Error("Fallo al enviar a Socket: %d,",socket);
			return 0;
		}
		cantEnviados += n;
		bytesleft -= n;
		//printf("Cantidad Enviada :%lu\n",n);
	}
	if(cantEnviados!=tamanioBuffer){
		return 0;
	}
	log_info(logger, "ENVIO DATOS. socket: %d. Cantidad Enviada:%lu ",socket,tamanioBuffer);
	return tamanioBuffer;
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

#if 1 // METODOS MANEJO DE ERRORES //
void Error(const char* mensaje, ...) {
	char* nuevo;
	va_list arguments;
	va_start(arguments, mensaje);
	nuevo = string_from_vformat(mensaje, arguments);

	fprintf(stderr, "\nERROR: %s\n", nuevo);
	log_error(logger, "%s", nuevo);

	va_end(arguments);
	if (nuevo != NULL )
		free(nuevo);
}
#endif

char* obtenerSubBuffer(char *nombre){
	// Esta funcion recibe un nombre y devuelve ese nombre de acuerdo al protocolo. Ej: carlos ------> 16carlos
	char *aux=string_new();
	int tamanioNombre=0;
	float tam=0;
	int cont=0;

	tamanioNombre=strlen(nombre);
	tam=tamanioNombre;
	while(tam>=1){
		tam=tam/10;
		cont++;
	}
	string_append(&aux,string_itoa(cont));
	string_append(&aux,string_itoa(tamanioNombre));
	string_append(&aux,nombre);

	return aux;
}
