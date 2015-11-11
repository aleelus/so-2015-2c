#include "conexion.h"

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
	socket_Memoria = (int) arg;

	printf("Se conecto la Memoria\n");

	//Es el ID del programa con el que está trabajando actualmente el HILO.
	//Nos es de gran utilidad para controlar los permisos de acceso (lectura/escritura) del programa.
	//(en otras palabras que no se pase de vivo y quiera acceder a una posicion de memoria que no le corresponde.)
	//	int id_Programa = 0;
	//	int tipo_Cliente = 0;

	// Es el encabezado del mensaje. Nos dice quien envia el mensaje
	int emisor = 0;

	// Dentro del buffer se guarda el mensaje recibido por el cliente.
	//char* buffer=string_new();
	char* buffer;

	// Cantidad de bytes recibidos.
	long unsigned bytesRecibidos;

	// La variable fin se usa cuando el cliente quiere cerrar la conexion: chau chau!
	int desconexionCliente = 0;

	// Código de salida por defecto
	int code = 0;
	while ((!desconexionCliente) && g_Ejecutando) {
		buffer = string_new();

		//Recibimos los datos del cliente
		bytesRecibidos = RecibirDatos(socket_Memoria,&buffer);

		if (bytesRecibidos > 0) {
			//Analisamos que peticion nos está haciendo (obtenemos el comando)
			emisor = ObtenerComandoMSJ(buffer);

			//Evaluamos los comandos
			switch (emisor) {
			case ADM_MEMORIA:
			{
				int orden;
				orden = PosicionDeBufferAInt(buffer, 1);
				ejecutarOrden(orden, buffer);

				mostrarParticionSwap();
//				printf("Hola Memoria\n");
//				EnviarDatos(socket_Memoria,"Ok",strlen("Ok"), COD_ADM_SWAP);
				break;
			}
			case COMANDO:
				printf("Ejecutado por telnet");
				EnviarDatos(socket_Memoria,"Ok",strlen("Ok"), COD_ADM_SWAP);
				break;
			default:
				procesarBuffer(buffer,bytesRecibidos);
				//free(buffer);
				//buffer = string_new();
				enviarArchivo();
				break;
			}
		}
		else {
			desconexionCliente = 1;
		}
		free(buffer);
	}
	CerrarSocket(socket_Memoria);
	return code;
}

void procesarBuffer(char* buffer, long unsigned tamanioBuffer){
	FILE * archivo = fopen("texto.txt","w");
	fwrite(buffer,1,tamanioBuffer,archivo);
	fclose(archivo);
}

void enviarArchivo(){
	FILE * archivo = fopen("texto2.txt","r");
	char * contenido;
	fseek(archivo,0L,SEEK_END);
	long unsigned tamanioA = ftell(archivo);

	contenido = malloc(tamanioA+1);
	memset(contenido,0,tamanioA+1);
	rewind(archivo);
	fread(contenido,1,tamanioA,archivo);

	EnviarDatos(socket_Memoria,contenido,tamanioA, COD_ADM_SWAP);

	free(contenido);
	fclose(archivo);
}


void EnviarRespuesta(operacion, fallo, pid){
	char* rsp = string_new();
		switch(operacion){
			case CREA_PROCESO:
			{
				//string_append(&rsp,"4111");
				char *aux = malloc(2);
				sprintf(aux,"%d",(fallo ? INIT_FAIL : INIT_OK));
				string_append(&rsp, aux);
				free(aux);
				break;
			}
			case SOLICITA_MARCO:
			{//OJO. en este caso el "fallo" es en realidad el numero de pagina que quier
				//char* fail = string_new();
				//string_append(&fail, "11");
				//string_append(&fail,READ_FAIL );

				//string_append(&rsp,"42");
				char *aux = realloc(rsp, __sizePagina__+1);
				if (aux == NULL){
					ErrorFatal("No se puede reallocar esta bosta");
				}
				rsp = aux;
				int paginaSolicitada = fallo;/*Esto es innecesario, lo hago por un tema de comprension :)*/
				int ptr = getPtrPaginaProcesoSolic(pid, paginaSolicitada);
				char* contenido = getContenido(ptr);
				aux = malloc(__sizePagina__+1);
				sprintf(aux, "%d",READ_FAIL );
				//string_append(&rsp, contenido != NULL ? contenido+getCorrimiento(ptr) : aux);
				memcpy(rsp, contenido != NULL ? contenido+getCorrimiento(ptr) : aux, __sizePagina__ );
				free(aux);
				if(contenido != NULL){
					log_info(logger, "Lectura de contenido mProc. PID: %d, Byte Inicial: %p, Tamaño del contenido: %d, Contenido: %s", pid, ptr , __sizePagina__, rsp);
					munmap(contenido, getTamanioPagina(ptr));
					EnviarDatos(socket_Memoria, rsp, __sizePagina__, COD_ADM_SWAP);
					free(rsp);
					return;
				}
				else
					Error("Fallo al leer contenido. PID: %d, Pagina solicitada: %d", pid, paginaSolicitada);
				break;
			}
			case REEMPLAZA_MARCO:
			{
			//	string_append(&rsp,"4311");
				char *aux = malloc(2);
				sprintf(aux, "%d", fallo ?  WRITE_FAIL : WRITE_OK );
				string_append(&rsp, aux);
				free(aux);
				break;
			}
			case FINALIZAR_PROCESO:
			{
				//string_append(&rsp,"4411");
				char *aux = malloc(2);
				sprintf(aux, "%d", fallo ? FIN_FAIL : FIN_OK);
				string_append(&rsp, aux);
				free(aux);
				break;
			}
		}

		EnviarDatos(socket_Memoria, rsp, strlen(rsp), COD_ADM_SWAP);
		free(rsp);
}
