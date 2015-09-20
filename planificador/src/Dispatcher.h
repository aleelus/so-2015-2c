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

extern sem_t semReady, semLock, semPCB;
extern t_list *PCBs, *colaReady, *colaBloqueados;

void Dispatcher(void *args);
t_PCB* algorimoFIFO();
t_PCB* algoritmoRoundRobin();
#endif /* DISPATCHER_H_ */
