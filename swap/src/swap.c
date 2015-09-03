/*o
 ============================================================================
 Name        : swap.c
 Author      : SO didn't C that coming
 Version     : 1.0
 Copyright   : SO didn't C that coming - UTN FRBA 2015
 Description : Trabajo Practivo Sistemas Operativos 2C 2015
 Testing	 :
 ============================================================================
 */

#include "swap.h"

int main(void) {
	//Si el tercer parametro es true graba en archivo y muestra en pantalla sino solo graba en archivo
	logger = log_create(NOMBRE_ARCHIVO_LOG, "swap", false, LOG_LEVEL_TRACE);

	// Levantamos el archivo de configuracion.
	LevantarConfig();

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

	return 0;
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
	char* buffer=string_new();

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
		bytesRecibidos = RecibirDatos(socket_Memoria,&buffer);

		if (bytesRecibidos > 0) {
			//Analisamos que peticion nos está haciendo (obtenemos el comando)
			emisor = ObtenerComandoMSJ(buffer);

			//Evaluamos los comandos
			switch (emisor) {
			case ES_MEMORIA:
				printf("Hola Memoria\n");
				EnviarDatos(socket_Memoria,"Ok",strlen("Ok"), YO);
				break;
			case COMANDO:
				printf("Ejecutado por telnet");
				EnviarDatos(socket_Memoria,"Ok",strlen("Ok"), YO);
				break;
			default:
				procesarBuffer(buffer,bytesRecibidos);
				free(buffer);
				buffer = string_new();
				enviarArchivo();
				break;
			}
		} else
			desconexionCliente = 1;
	}
	CerrarSocket(socket_Memoria);
	return code;
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

	EnviarDatos(socket_Memoria,contenido,tamanioA, YO);

	free(contenido);

	fclose(archivo);
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
		if (config_has_property(config, "NOMBRE_SWAP")) {
			g_Nombre_Swap = config_get_string_value(config, "NOMBRE_SWAP");
		} else
			Error("No se pudo leer el parametro NOMBRE_SWAP");
		if (config_has_property(config, "CANTIDAD_PAGINAS")) {
			g_Cantidad_Paginas = config_get_int_value(config, "CANTIDAD_PAGINAS");
		} else
			Error("No se pudo leer el parametro CANTIDAD_PAGINAS");
		if (config_has_property(config, "TAMANIO_PAGINA")) {
			g_Tamanio_Pagina = config_get_int_value(config, "TAMANIO_PAGINA");
		} else
			Error("No se pudo leer el parametro TAMANIO_PAGINA");
		if (config_has_property(config, "RETARDO_COMPACTACION")) {
			g_Retardo_Compactacion = config_get_int_value(config, "RETARDO_COMPACTACION");
		} else
		Error("No se pudo leer el parametro RETARDO_COMPACTACION");
	} else {
		ErrorFatal("No se pudo abrir el archivo de configuracion");
	}
	if (config != NULL ) {
		free(config);
	}
}

#endif

void Comenzar_Consola() {

	int corte_consola = -1;
	printf("Consola para ingresar Comandos al planificador\n");
	while (corte_consola != 0) {
		corte_consola = operaciones_consola();
	}
	printf("Se termino la ejecucion de la consola del filesystem\n");
}

int operaciones_consola() {

	int variable_seleccion;
	scanf("%d", &variable_seleccion);
	printf("Comando incorrecto.\n");
	return 0;
}
