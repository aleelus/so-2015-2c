/*
 * api.c
 *
 *  Created on: 30/8/2015
 *      Author: utnso
 */

#include "api.h"

int conectarCliente (char* ip, char* puerto) {
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(ip, puerto, &hints, &serverInfo);
	int serverSocket;
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	if (serverSocket==-1){
		//log_error(logs, "Error al crear el socket");
		printf("Error al crear el socket");
		return 0;
	}
	if (connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen)){
		//log_error(logs, "Error al conectar el socket");
		printf("Error al conectar el socket");
		close(serverSocket);
		return 0;
	}
	freeaddrinfo(serverInfo);
	return serverSocket;
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

	free(bufferAux);
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

long unsigned EnviarDatos(int socket, char *buffer, long unsigned tamanioBuffer, char* YO) {
	//Tenia que agregarle el parametro porque en api.h no puedo poner muchos #defines :P

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
	log_trace(logger, "ENVIO DATOS. socket: %d. Cantidad Enviada:%lu ",socket,tamanioBuffer);

	free(bufferE);
	free(bufferR);
	return tamanioBuffer;
}


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
int cuentaDigitos(int valor){
	int cont = 0;
	float tamDigArch=valor;
	while(tamDigArch>=1){
		tamDigArch=tamDigArch/10;
		cont++;
	}
	return cont;
}

char* obtenerSubBufferDeContenido(char *nombre,int tamanio){

	//Le tengo q pasar un string y un tamanio,
	//El nombre debe tener el resto del contenido con \0, Ej: nombre="hola\0\0\0....\0" la cantidad de \0 son (tamanio-strlen(hola)) PD: Puse el strlen como ejemplo pero no se puede usar xDXDXDXDxXxXdXXdXDXd
	//Ej: nombre= AhiEstaElYetaDeCici\0,\0aTocarMaderaTodos  tamanio=256  ==> salida= 3256AhiEstaElYetaDeCici\0,\0aTocarMaderaTodos\0\0\0\0...\0

	float tam=tamanio;
	int cont=0;
	char *aux;

	while(tam>=1){
		tam=tam/10;
		cont++;
	}

	aux = malloc(tamanio+cuentaDigitos(cont)+cuentaDigitos(tamanio));
	memset(aux,0,tamanio+cuentaDigitos(cont)+cuentaDigitos(tamanio));


	memcpy(aux,string_itoa(cont),strlen(string_itoa(cont)));   // Son nros, puedo hacer stlren
	memcpy(aux+strlen(string_itoa(cont)),string_itoa(tamanio),strlen(string_itoa(tamanio)));   // Son nros, puedo hacer stlren
	memcpy(aux+strlen(string_itoa(cont))+strlen(string_itoa(tamanio)),nombre,tamanio);



	return aux;
}

void imprimirContenido(char* contenido, long unsigned tamanio){
	long unsigned i=0;
	while(i<tamanio){
		if(*(contenido+i)!='\0'){
			printf("%c",*(contenido+i));
		} else {
			printf("\\0");
		}
		i++;
	}
}

int strlenHastaUnChar(char* string, char unChar){
	//LTABarraCero
	int length = 0;

	for(length = 0 ;string[length] != unChar;length++){
		//cuenta caracter por caracter
	}

	return length;
}
