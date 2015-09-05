/*o
 ============================================================================
 Name        : planificador.c
 Author      : SO didn't C that coming
 Version     : 1.0
 Copyright   : SO didn't C that coming - UTN FRBA 2015
 Description : Trabajo Practivo Sistemas Operativos 2C 2015
 Testing	 :
 ============================================================================
 */

#include "planificador.h"


int main(void) {
	//Si el tercer parametro es true graba en archivo y muestra en pantalla sino solo graba en archivo
	logger = log_create(NOMBRE_ARCHIVO_LOG, "planificador", false, LOG_LEVEL_TRACE);

	lista_comandos = list_create();
	lista_cpu = list_create();

	CargarListaComandos();

	sem_init(&semListaCpu,0,1);

	// Levantamos el archivo de configuracion.
	LevantarConfig();

	int iThreadConsola;					//Hilo de consola

	//Hilo orquestador conexiones
	int iThreadOrquestador = pthread_create(&hOrquestadorConexiones, NULL,
			(void*) HiloOrquestadorDeConexiones, NULL );
	if (iThreadOrquestador) {
		fprintf(stderr,
			"Error al crear hilo - pthread_create() return code: %d\n",
			iThreadOrquestador);
		exit(EXIT_FAILURE);
	}

	//Este hilo es el que maneja la consola
	if ((iThreadConsola = pthread_create(&hConsola, NULL, (void*) Comenzar_Consola, NULL)) != 0){
		fprintf(stderr,
			"Error al crear hilo - pthread_create() return code: %d\n",
			iThreadConsola);
		exit(EXIT_FAILURE);
	};

	pthread_join(hOrquestadorConexiones, NULL );
	pthread_join(hConsola, NULL );

	return 0;
}

void CargarListaComandos(){
	list_add(lista_comandos,"correr");
	list_add(lista_comandos,"finalizar");
	list_add(lista_comandos,"ps");
	list_add(lista_comandos,"cpu");
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


int AtiendeCliente(void * arg) {

	t_cpu * cpu = malloc(sizeof(t_cpu));

	cpu->socket_Cpu =  (int) arg;;
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

	printf(
			"El programa se cerrara. Presione ENTER para finalizar la ejecución.");
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
	if(cantEnviados!=tamanioBuffer) return 0;
	log_info(logger, "ENVIO DATOS. socket: %d. Cantidad Enviada:%lu ",socket,tamanioBuffer);
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
		if (config_has_property(config, "ALGORITMO_PLANIFICACION")) {
			g_Algoritmo = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
		} else
			Error("No se pudo leer el parametro ALGORITMO_PLANIFICACION");
		if (config_has_property(config, "QUANTUM")) {
			g_Quantum = config_get_int_value(config, "QUANTUM");
		} else
			Error("No se pudo leer el parametro PUERTO");

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

void Comenzar_Consola() {

	int corte_consola = -1;
	printf("Consola para ingresar Comandos al planificador\n");
	while (corte_consola != 0) {
		corte_consola = operaciones_consola();
	}
	printf("Se termino la ejecucion de la consola del filesystem\n");
}

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

void imprimirComandos(char* comando){
	int i=0;
	char * com;
	printf("\n");
	while(i<list_size(lista_comandos)){
		com = list_get(lista_comandos,i);
		if(esParecido(comando,com)){
			printf(" %s ",com);
		}
		i++;
	}
	printf("\n%s",comando);
}

void enviarArchivo(){
	FILE * archivo = fopen("texto.txt","r");

	t_cpu* cpu;

	char * contenido;

	fseek(archivo,0L,SEEK_END);

	long unsigned tamanioA = ftell(archivo);

	contenido = malloc(tamanioA+1);
	memset(contenido,0,tamanioA+1);

	rewind(archivo);

	fread(contenido,1,tamanioA,archivo);

	cpu = list_get(lista_cpu,0);

	EnviarDatos(cpu->socket_Cpu,contenido,tamanioA);

	free(contenido);

	sem_post(&cpu->semaforo);
}

int procesarComando(comando){
	enviarArchivo();
	return 1;
}

int operaciones_consola() {
	int c;
	scanf("%d",&c);
	procesarComando("prueba");
	return 1;
}

void procesarBuffer(char* buffer, long unsigned tamanioBuffer){

	FILE * archivo = fopen("texto2.txt","w");

	fwrite(buffer,sizeof(char),tamanioBuffer,archivo);

	fclose(archivo);
}
