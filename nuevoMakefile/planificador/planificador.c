/*o
 ============================================================================
 Name        : planificador.c
 Author      : SO didn't C that coming
 Version     : 1.0
 Copyright   : SO didn't C that coming - UTN FRBA 2015
 Description : Trabajo Practivo Sistemas Operativos 2C 2015
 Testing	 :
 ============================================================================
 */

#include "planificador.h"





t_list *PCBs, *colaReady, *colaBloqueados;

pthread_t hOrquestadorConexiones, hConsola, hDispatcher;

int cantHilos=0;
//int PID=0; // Contador de ID Procesos mProc (semaforo)
t_pid PID; // Contador ID Procesos + semaforo ;)

sem_t semPCB, semReady, semLock, semIO;



int main(void) {

	//Si el tercer parametro es true graba en archivo y muestra en pantalla sino solo graba en archivo
	logger = log_create(NOMBRE_ARCHIVO_LOG, "planificador", false, LOG_LEVEL_TRACE);
	pthread_mutex_init(&lockLogger, NULL);

	lista_cpu = list_create();

	//CargarListaComandos();

	/* Inicializar semaforos */
	sem_init(&semListaCpu,0,1);
	inicializarListas();


	// Levantamos el archivo de configuracion.
	LevantarConfig();

	int iThreadConsola;					//Hilo de consola

	//Hilo orquestador conexiones
	int iThreadOrquestador = pthread_create(&hOrquestadorConexiones, NULL, (void*) HiloOrquestadorDeConexiones, NULL );
	if (iThreadOrquestador) {
		fprintf(stderr, "Error al crear hilo - pthread_create() return code: %d\n", iThreadOrquestador);
		exit(EXIT_FAILURE);
	}

	//Este hilo es el que maneja la consola
	if ((iThreadConsola = pthread_create(&hConsola, NULL, (void*) ComenzarConsola, NULL)) != 0){
		fprintf(stderr, "Error al crear hilo - pthread_create() return code: %d\n", iThreadConsola);
		exit(EXIT_FAILURE);
	}

	pthread_join(hOrquestadorConexiones, NULL );
	pthread_join(hConsola, NULL );
	log_destroy(logger);
	return 0;
}

int crearProceso(char* path) {
	if (access(path, F_OK)){
		Error("El path %s no se corresponde a un archivo existente.", path);
		return 0;
	}
	t_PCB *nuevoPCB = malloc(sizeof(t_PCB));
	nuevoPCB->estado = LISTO;
	nuevoPCB->horaCreacion = time(NULL);
	nuevoPCB->nroLinea = 0;
	nuevoPCB->path = malloc(strlen(path)+1);
	strcpy(nuevoPCB->path, path);
	nuevoPCB->pid = nuevoPid();
	nuevoPCB->quantum = g_Quantum;

	sem_wait(&semPCB);
	sem_wait(&semReady);
	list_add(PCBs, nuevoPCB);
	list_add(colaReady, nuevoPCB);
	sem_post(&semReady);
	sem_post(&semPCB);
	return nuevoPCB->pid;
}

int finalizarProceso(int pid) {

	bool _mismoPID(t_PCB *pcb){
		return pcb->pid == pid;
	}
	bool _cpuEjecutando(t_cpu *cpu){
		return _mismoPID(cpu->procesoAsignado);
	}

	sem_wait(&semPCB);
	sem_wait(&semReady);
	sem_wait(&semLock);

	t_cpu *cpu = NULL;
	t_PCB *proceso = list_find(PCBs, (void*) _mismoPID );
	t_PCB *aux = proceso;
	if (proceso == NULL){
		fprintf(stderr, "El proceso %d no existe", pid);
		return 1;
	}
	if(proceso->estado == EJECUTANDO){
		//TODO: Sacarlo de la CPU, matarlo y enterrar el cadaver donde nadie pueda verlo
		sem_wait(&semListaCpu);
		cpu = list_find(lista_cpu, (void*)_cpuEjecutando);
		sem_post(&semListaCpu);
		sem_wait(&cpu->semaforoProceso); // TODO: Revisar, podria causar deadlock


	}
	if(proceso->estado == LISTO) {
		aux = list_find(colaReady, (void*) _mismoPID);
	}
	if(proceso->estado == BLOQUEADO || proceso->estado == ESPERANDO_IO){

		aux = list_find(colaBloqueados, (void*) _mismoPID);
	}
	if (aux != proceso){
		fprintf(stderr, "Error irrecuperable al finalizar el proceso %d\n", pid);
		if(__DEBUG__){
			fprintf(stderr, "Punteros: %d, %d", aux, proceso);
		}
		exit(0);
	}

	proceso->nroLinea =-1;

	if (cpu != NULL) sem_post(&cpu->semaforoProceso);
	sem_post(&semLock);
	sem_post(&semReady);
	sem_post(&semPCB);

	return proceso;
}

int nuevoPid(){
	sem_wait(&PID.sem);
	int pidAsignado = PID.pid++; //Aumento el pid en uno y lo guardo en una variable local
	sem_post(&PID.sem);
	return pidAsignado;
}

void mostrarProcesos(FILE* salida){
	t_PCB* proceso;
	fprintf(salida, "|%-30s||%-30s||%-30s\n", "PID", "ESTADO", "PATH");
	int i;
	sem_wait(&semPCB);
	for(i=0;i<list_size(PCBs);i++){
		proceso = list_get(PCBs,i);
		fprintf(salida, "|%-30d||%-30s||%-30s\n", proceso->pid, estados[proceso->estado], proceso->path);
	}
	sem_post(&semPCB);
}

/*void loguearEstadoProcesos(){
	t_PCB* proceso;
	log_info(logger, "|%-30s||%-30s||%-30s\n", "PID", "ESTADO", "PATH");
	int i;
	sem_wait(&semPCB);
	for(i=0;i<list_size(PCBs);i++){
		proceso = list_get(PCBs,i);
		log_info(logger, "|%-30d||%-30s||%-30s\n", proceso->pid, estados[proceso->estado], proceso->path);
	}
	sem_post(&semPCB);
}*/


void inicializarListas() {
	sem_init(&PID.sem, 0, 1);
	sem_init(&semPCB, 0, 1);
	sem_init(&semReady, 0, 1);
	sem_init(&semLock, 0, 1);
	sem_init(&semIO, 0, 1);

	PID.pid = 1;
	PCBs = list_create();
	colaReady = list_create();
	colaBloqueados = list_create();
}

void ejecutarDispatcher(){
	int *algoritmo = malloc(sizeof(int*));
	if (algoritmo == NULL){
		Error("Error al crear hilo de dispatcher");
		return;
	}
	*algoritmo = g_Algoritmo;
	int iThreadDispatcher = pthread_create(&hDispatcher, NULL, (void*) Dispatcher, algoritmo );
	if ( iThreadDispatcher != 0 ){
		fprintf(stderr, "Error al crear hilo dispatcher - pthread_create() return code: %d\n", iThreadDispatcher);
		exit(EXIT_FAILURE);
	}
}

void mostrarPorcentajesDeUso(){
	sem_wait(&semListaCpu);
	int i;
	printf("Porcentaje de uso:\n");
	for(i=0;i<lista_cpu->elements_count;i++){
		t_cpu* cpu = list_get(lista_cpu,i);
		sem_wait(&cpu->semaforoProceso);
		sem_wait(&cpu->semUso);
		bit_array* aux = malloc(sizeof(bit_array));
		*aux = *(cpu->uso);

		if(cpu->procesoAsignado != NULL && cpu->procesoAsignado->estado == EJECUTANDO){
			llenarUnos(aux, (int64_t)(difftime(time(NULL), cpu->horaEntrada)));
		}
		else {
			llenarZeros(aux, (int64_t)(difftime(time(NULL), cpu->horaSalida)));
		}
		if(__DEBUG__){
			imprimirBinario(*aux);
			printf("\n");
		}
		sem_post(&cpu->semUso);
		sem_post(&cpu->semaforoProceso);

		printf(" - CPU %d: %.2f\%\n", i+1, 100*(double)sumarBits(*aux)/60);
		free(aux);
	}
	sem_post(&semListaCpu);
}
