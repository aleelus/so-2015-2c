// Bibliotecas //

#include "Consola.h"
//#ifndef UTILIDADES_UTILS_H
//#include "Utilidades/Utils.h"
//#endif

#undef getc



#define PLANIFICADOR "1"
#define BUFFERSIZE 50
#define __DEBUG__ 1
//Tipos de operaciones
#define ES_CPU	2
#define COMANDO 8
#define TEXTO2	9






typedef struct {
	int pid;
	int nroLinea; // Ultima linea = -1
	int quantum;
	t_estado estado;
	time_t horaCreacion;
	char* path;
} t_PCB;

