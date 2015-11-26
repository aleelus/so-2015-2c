/*
 * Dispatcher.h
 *
 *  Created on: 20/9/2015
 *      Author: utnso
 */

#ifndef DISPATCHER_H_
#define DISPATCHER_H_
#ifndef UTILIDADES_UTILS_H_
#include "Utilidades/Utils.h"
#endif

extern sem_t semReady, semLock, semPCB, semIO;
extern t_list *PCBs, *colaReady, *colaBloqueados;

typedef struct {
	int pid;
	int tiempo;
}t_noni;


void Dispatcher(void *args);
void pasarABloqueados(t_cpu* cpu, int tiempo, int proximaInstruccion);
void pasarAReady(t_cpu* cpu, int proximaInstruccion);
void dormirProceso(t_noni* noni);
t_PCB* algorimoFIFO();
t_PCB* algoritmoRoundRobin();
#endif /* DISPATCHER_H_ */
