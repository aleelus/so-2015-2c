// Bibliotecas //
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <semaphore.h>
#include <netdb.h>
#include <api.h>
#include <protocolo.h>
#include <netdb.h>
#include <sys/types.h>
#include <string.h>
#include "configuracion.h"
#include "conexion.h"
#include "particionSwap.h"

#define COLOR_VERDE   "\x1b[32m"
#define DEFAULT   "\x1b[0m"
#define YO	"4"
//**Define para mensajes recibidos de MEMORIA**/
#define CREA_PROCESO 1
#define SOLICITA_MARCO 2
#define REEMPLAZA_MARCO 3
#define FINALIZAR_PROCESO 4
#define ENTREGA1 0 //Dejar en 1 hasta la entrega 1

/*Lista de bloques globales*/
t_list* listaBloquesLibres;
t_list* listaBloquesOcupados;

//sem_t semaforoListaNodos,semaforoListaArchivos,semaforoListaJobEnviados,semIdJob,semSocket;

// CONSTANTES //
//Ruta del config
#define PATH_CONFIG "config.cfg"
#define NOMBRE_ARCHIVO_LOG	"swap.log"

//Tamaño del buffer
#define BUFFERSIZE 50

//t_list* lista_nodos;

int socket_Memoria;

//Parametros de archivo de configuracion
int g_Puerto;
char* g_Nombre_Swap;
int g_Cantidad_Paginas;
int g_Tamanio_Pagina;
int g_Retardo_Compactacion;

// METODOS MANEJO DE ERRORES //
void Error(const char* mensaje, ...);

// Logger del commons
t_log* logger;

//Contador de Hilos
int cantHilos;

// Definimos los hilos principales
pthread_t hOrquestadorConexiones, hConsola;

// - Bandera que controla la ejecución o no del programa. Si está en 0 el programa se cierra.
int g_Ejecutando;

//Tipos de operaciones
#define COMANDO 8

//Funciones
int ejecutarOrden(int, char*);
