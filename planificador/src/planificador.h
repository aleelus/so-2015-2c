// Bibliotecas //

#include "Consola.h"
#include "Dispatcher.h"
#ifndef UTILIDADES_UTILS_H_
#include "Utilidades/Utils.h"
#endif

#undef getc


#define PLANIFICADOR "1"
#define BUFFERSIZE 50
//#define __DEBUG__ 1
//Tipos de operaciones
#define ES_CPU	2
#define COMANDO 8
#define TEXTO2	9



/*************************ESTRUCTURAS**************************/




typedef struct {
	int pid;
	sem_t sem;
} t_pid;
