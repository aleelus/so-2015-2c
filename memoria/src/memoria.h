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
#include <signal.h>

#define COLOR_VERDE   "\x1b[32m"
#define COLOR_CYAN "\x1b[36m"
#define COLOR_MAGENTA "\x1b[35m"
#define DEFAULT   "\x1b[0m"
#define YO	"3"

//Tipo de Operacion con CPU
#define INICIAR 1
#define LEER 2
#define ESCRIBIR 3
#define FINALIZAR 4


sem_t semTLB,semMP,semSwap,semLog;

pthread_mutex_t semMemPrincipal = PTHREAD_MUTEX_INITIALIZER;

// CONSTANTES //
//Ruta del config
#define PATH_CONFIG "config.cfg"
#define NOMBRE_ARCHIVO_LOG	"memoria.log"

//Tamaño del buffer
#define BUFFERSIZE 50

//Parametros del archivo de configuracion
int g_Puerto;
char* g_Ip_Swap;
char* g_Puerto_Swap;
int g_Maximo_Marcos_Por_Proceso;
int g_Cantidad_Marcos;
int g_Tamanio_Marco;
int g_Entradas_TLB;
char* g_TLB_Habilitada;
int g_Retardo_Memoria;
char* g_Algoritmo;

// METODOS CONFIGURACION //
void LevantarConfig();

// METODOS MANEJO DE ERRORES //
void Error(const char* mensaje, ...);

// Logger del commons
t_log* logger;

//Contador de Hilos
int cantHilos=0;

//Socket Swap
int socket_Swap;

// Definimos los hilos principales
pthread_t hCrearHilos, hOrquestadorConexiones,hSeniales;

//METODOS MANEJO SOCKETS
void HiloOrquestadorDeConexiones();

char* obtenerSubBuffer(char *);
int cuentaDigitos(int );

// - Bandera que controla la ejecución o no del programa. Si está en 0 el programa se cierra.
int g_Ejecutando = 1;

//Tipos de operaciones
#define ES_CPU	2
#define ES_SWAP 4
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
long unsigned EnviarDatos(int socket, char *buffer, long unsigned tamanioBuffer);
void LevantarConfig();
void Error(const char* mensaje, ...);
char* obtenerSubBuffer(char *nombre);
int conexionASwap();
void iniciarTLB();
void iniciarListamProc();
void iniciarMemoriaPrincipal();
void Manejador(int signum);
void Seniales();
void funcionLimpiarTablasPaginas();
void funcionBuscarPidPagina(int marco,int * pid, int * pagina);
int grabarContenidoASwap(int pid,int nroPagina,char* contenido);
void imprimirContenido(char* contenido, long unsigned tamanio);
//Estructuras

//Pagina
typedef struct {
	int pagina;
	int marco;
	int bitMP;
} t_pagina;

//TLB
typedef struct {
	int pagina;
	int marco;
	int pid;
} t_tlb;

t_list* lista_tlb;

//mProc
typedef struct {
	int pid;
	int cantMarcosPorProceso;
	t_list* paginas;
} t_mProc;

t_list* lista_mProc;

//Memoria Principal
typedef struct {
	int pag;
	int bitModificado; //Algoritmo
	int bitUso;	       //Algoritmo
	int marcoEnUso;   // Interno Nuestro
	int bitPuntero;
	int pid;
	char* contenido;
} t_mp;

typedef struct {
	int pagina;
	int pid;
} t_lru;

t_list *lista_lru;

t_mp * a_Memoria;

