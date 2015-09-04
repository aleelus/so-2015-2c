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
#include <commons/config.h>
#include <string.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <semaphore.h>
#include <netdb.h>
#include <api.h>
#include <protocolo.h>

#define COLOR_VERDE   "\x1b[32m"
#define DEFAULT   "\x1b[0m"
#define YO	"4"


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

// METODOS CONFIGURACION //
void LevantarConfig();

// METODOS MANEJO DE ERRORES //
void Error(const char* mensaje, ...);

// Logger del commons
t_log* logger;

//Contador de Hilos
int cantHilos=0;

// Definimos los hilos principales
pthread_t hOrquestadorConexiones, hConsola;

//METODOS MANEJO SOCKETS
void HiloOrquestadorDeConexiones();

char* obtenerSubBuffer(char *);
int cuentaDigitos(int );

// - Bandera que controla la ejecución o no del programa. Si está en 0 el programa se cierra.
int g_Ejecutando = 1;

//Tipos de operaciones
#define ES_MEMORIA	3
#define COMANDO 8

//Funciones
void CerrarSocket(int socket);
int AtiendeCliente(void * arg);
void ErrorFatal(const char* mensaje, ...);
int ObtenerComandoMSJ(char* buffer);
int PosicionDeBufferAInt(char* buffer, int posicion);
int ChartToInt(char x);
char* DigitosNombreArchivo(char *buffer,int *posicion);
long unsigned RecibirDatos(int socket,char** buffer);
void LevantarConfig();
void Error(const char* mensaje, ...);
char* obtenerSubBuffer(char *nombre);
void Comenzar_Consola();
int operaciones_consola();
