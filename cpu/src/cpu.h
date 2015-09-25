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

#define COLOR_VERDE   "\x1b[32m"
#define DEFAULT   "\x1b[0m"
#define YO	"2"

static __thread int socketPlanificador;
sem_t semDormilon;

// CONSTANTES //
//Ruta del config
#define PATH_CONFIG "config.cfg"
#define NOMBRE_ARCHIVO_LOG	"cpu.log"

//Tamaño del buffer
#define BUFFERSIZE 50

// METODOS CONFIGURACION //
void LevantarConfig();

// METODOS MANEJO DE ERRORES //
void Error(const char* mensaje, ...);

//METODOS MANEJO SOCKETS
void HiloOrquestadorDeConexiones();

char* obtenerSubBuffer(char *);
int cuentaDigitos(int );

//Tipos de operaciones
#define ES_PLANIFICADOR	1
#define ES_MEMORIA 3
#define COMANDO 8
#define TEXTO1 7

//Funciones
int AtiendeCliente(void * arg);
void LevantarConfig();
void CrearHilos();
void ProcesoCPU();
void ConectarPlanificador(int* socket_Planificador);
void CrearCPU();
void inicializarListaDeProcesos();
void inicializarVectorDeSockets();
