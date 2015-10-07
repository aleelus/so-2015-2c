#include "swap.h"
#include <stdlib.h>
#define __TEST_FRAGMENTACION__ 0

 g_Ejecutando = 1;
 cantHilos=0;

int main(void) {
	//Si el tercer parametro es true graba en archivo y muestra en pantalla sino solo graba en archivo
	logger = log_create(NOMBRE_ARCHIVO_LOG, "swap", true, LOG_LEVEL_TRACE);

	// Levantamos el archivo de configuracion.
	LevantarConfig();


		if(!crearParticionSwap())
			ErrorFatal("Error al crear la particion de swap");
		else{
			if(crearEstructuraBloques() < 0 )
				ErrorFatal("Error al crear las estructuras de bloques de swap");
			if(__TEST_FRAGMENTACION__){
				crearEntornoParaTestDesfragmentacion();
				ejecutarOrden(1, "311225112");
			}
		}
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

void crearEntornoParaTestDesfragmentacion(){
	// FILE *ptr;
	// getComienzoParticionSwap(&ptr);
	int ptr  = 0;
	if(ptr!=NULL){
		list_remove(listaBloquesLibres, 0);
		list_add(listaBloquesLibres, t_block_free_create(ptr, 1));
		list_add(listaBloquesLibres, t_block_free_create(ptr + 2*__sizePagina__, 1));
		list_add(listaBloquesOcupados, t_block_used_create(24, ptr + __sizePagina__, 1));
		list_add(listaBloquesOcupados, t_block_used_create(45, ptr + 3*__sizePagina__,3));
	}
	else
		Error("Error al crear test para desfragmentacion");
}

int crearParticionSwap(){
			int res;
			char* cmd = string_new();
			string_append(&cmd, "dd if=/dev/zero of=./");
			string_append(&cmd, g_Nombre_Swap);
			string_append(&cmd, ".bin bs=");
			string_append(&cmd, string_itoa(__sizePagina__));
			string_append(&cmd, " count=");
			string_append(&cmd, string_itoa(g_Cantidad_Paginas));
			if((res = system(cmd))== -1)
			{
				perror("Error al intentar crear particion de swap");
				free(cmd);
				return -1;
			}
			free(cmd);
			return 1;
}


int ejecutarOrden(int orden, char* buffer){
	switch(orden){
	case CREA_PROCESO:
	{
		int posActual = 2;
		int pid = strtol(DigitosNombreArchivo(buffer,&posActual), NULL, 10);
		int paginasSolicitadas = strtol(DigitosNombreArchivo(buffer, &posActual), NULL, 10);
		int res = crearProceso(pid, paginasSolicitadas);
		EnviarRespuesta(CREA_PROCESO, (res < 0) ? __FALLO__ : guardarEnBloque(paginasSolicitadas, pid), NULL);
		//free(pid);
		//free(paginasSolicitadas);

	}
		break;
	case SOLICITA_MARCO:
	{
		int posActual = 2;
		int pid = strtol(DigitosNombreArchivo(buffer,&posActual), NULL, 10);
		int paginaSolicitada = strtol(DigitosNombreArchivo(buffer, &posActual), NULL, 10);
		EnviarRespuesta(SOLICITA_MARCO, paginaSolicitada, pid);
		break;
	}
	case REEMPLAZA_MARCO:{
		int res = reemplazarMarco(buffer) ;
		EnviarRespuesta(REEMPLAZA_MARCO, (res < 0) ? __FALLO__ : __PROC_OK__, NULL);

		break;
	}
	case FINALIZAR_PROCESO:{
		int res = finalizarProceso(buffer);
		EnviarRespuesta(FINALIZAR_PROCESO, (res < 0) ? __FALLO__ :__PROC_OK__, NULL);
		break;
		}
	}
	//free(buffer); // No hay que liberar aca
}

int finalizarProceso(char* buffer){
	int posActual = 2;
	int pid = strtol(DigitosNombreArchivo(buffer,&posActual), NULL, 10);
	//long PID = atol(pid);
	//free(pid);

	bool _es_bloque_a_reemplazar(t_block_used* bloque)
		{
			return(bloque->pid == pid);
		}
		//Obtengo el bloque que contiene el marco a reemplazar.
		t_block_used *bloque = list_remove_by_condition(listaBloquesOcupados, (void*) _es_bloque_a_reemplazar );

		if (bloque != NULL){
/* 		//Obtengo la direccion del marco
			FILE* ptrComienzoParticion;
			int fd;
			char * dir = string_new();
			string_append(&dir, "/home/utnso/git/");
			string_append(&dir, g_Nombre_Swap);
			string_append(&dir, ".bin");
			ptrComienzoParticion = fopen(dir, "r");
			struct stat sbuf;

			if ((fd = open(dir, O_RDONLY)) == -1) {
				free(dir);
				Error("Error al finalizar proceso %d: no fue posible abrir la particion de swap", pid);
				return -1;
			}
			//Verifico el estado del archivo que estoy abriendo
			if (stat(dir, &sbuf) == -1) {
				free(dir);
				Error("Error al finalizar proceso %d: Verificar estado de archivo particion de swap", pid);
				return -1;
			}
			free(dir); */

			int fd = abrirParticionSwap();

			char *datos = mmap((caddr_t) 0, getTamanioPagina(bloque->ptrComienzo) * bloque->cantPag, PROT_WRITE, MAP_SHARED, fd, getOffset(bloque->ptrComienzo ));
			close(fd);
			if (datos == MAP_FAILED) {
					perror("mmap");
					Error("Error al obtener datos contenidos en la pagina");
					return NULL;
			}

			memset(datos+getCorrimiento(bloque->ptrComienzo), '\0',  __sizePagina__ * bloque->cantPag);

			int ret = msync(datos, __sizePagina__ * bloque->cantPag, MS_INVALIDATE);
			if(ret < 0){
				Error("Error al intentar sincronizar datos de pagina %d", pid);
				return -1;
			}
			ret = munmap( datos , getTamanioPagina(bloque->ptrComienzo) );
			if (ret < 0){
				ErrorFatal("Error al ejecutar munmap al intentar finalizar proceso PID: %d", pid);
			}

			list_add(listaBloquesLibres, t_block_free_create(bloque->ptrComienzo, bloque->cantPag));


			log_info(logger,"Proceso mProc liberado. PID: %d ; Byte Inicial: %p ; Tamaño Liberado(en bytes): %d ;", bloque->pid, bloque->ptrComienzo, bloque->cantPag );
			t_block_used_destroy(bloque);
			return 1;
		}
		else{
			Error("Error al intentar finalizar proceso %d: No se encontro el bloque correspondiente al mismo", pid);
			return -1;
		}

}

int abrirParticionSwap(){
	char * dir = string_new();
		string_append(&dir, "./");
		string_append(&dir, g_Nombre_Swap);
		string_append(&dir, ".bin");
		//FILE* ptrComienzoParticion = fopen(dir, "r+");
		struct stat sbuf;
		int fd;
		//if(ptrComienzoParticion == NULL)
		//	ErrorFatal("Error al abrir particion de swap");

		if ((fd = open(dir, O_RDWR)) == -1) {
			Error("Error al abrir la particion de swap");
			return NULL;
		}
		//Verifico el estado del archivo que estoy abriendo
		if (stat(dir, &sbuf) == -1) {
			Error("Error: Verificar estado de archivo particion de swap");
			return NULL;
		}
		free(dir);

		//return ptrComienzoParticion;
		return fd;
}


int reemplazarMarco(char* buffer){
	int posActual = 2;
	int pid = strtol(DigitosNombreArchivo(buffer,&posActual), NULL, 10);
	int pagina = strtol(DigitosNombreArchivo(buffer, &posActual), NULL, 10);
	//int pag = atoi(pagina);
	//long PID = atol(pid);
	//free (pagina);
	//free(pid);

	bool _es_bloque_a_reemplazar(t_block_used* bloque)
	{
		return(bloque->pid == pid);
	}
	//Obtengo el bloque que contiene el marco a reemplazar.
	t_block_used *bloque = list_find(listaBloquesOcupados, (void*) _es_bloque_a_reemplazar );

	if (bloque != NULL){
	//Obtengo la direccion del marco

		int pos = bloque->ptrComienzo + (pagina * __sizePagina__);
		int fd = abrirParticionSwap();
//		if (ptrComienzoParticion == NULL)
//		{
//			Error("Error al intentar reemplazar marco de PID: %d ", pid);
//			return -1;
//		}
//
//		if( fseek( ptrComienzoParticion, pos, SEEK_SET )< 0 ){
//			Error ("Error al reemplazar marco PID: %d ", pid);
//			return -1;
//		}

		char *datos = mmap((caddr_t) 0, getTamanioPagina(pos), PROT_WRITE, MAP_SHARED, fd, getOffset(pos) ); // TODO Crear funciones para esto
		close(fd);
		if (datos == MAP_FAILED) {
				perror("mmap");
				Error("Error al obtener datos contenidos en la pagina");
				return NULL;
		}

		memcpy(datos+getCorrimiento(pos), DigitosNombreArchivo(buffer, &posActual), __sizePagina__); //TODO Crear funcion para esto
		int ret = msync(datos, __sizePagina__, MS_INVALIDATE);
		if(ret < 0){
			Error("Error al intentar sincronizar datos de pagina %d", pid);
			return -1;
		}
		log_info(logger, "Escritura de contenido mProc. PID: %d, Byte Inicial: %p, Tamaño del contenido: %d, Contenido: %s", pid, pos, __sizePagina__, datos+getCorrimiento(pos) );
		ret = munmap( datos , getTamanioPagina(pos) ); // TODO Controlar cuanto munmapear
		if (ret < 0){
			ErrorFatal("Error al ejecutar munmap");
		}
		sleep(__retardoSwap__);
		return 1;
	}
	else{
		Error("No se encontro ningun bloque correspondiente al pid %d", pid);
		return -1;
	}
}




int crearProceso(int pid, int paginasSolicitadas){
	int totalPaginasLibres = getCantidadPaginasLibres();
	if(paginasSolicitadas > totalPaginasLibres){
		Error("Proceso mProc PID: %d rechazado por falta de espacio.", pid);
		return -1;
	}
	else
	{
		if(!existeEspacioContiguo(paginasSolicitadas))
			desfragmentar();

		return 1;
	}
}

int getPtrPaginaProcesoSolic(pid, paginaSolicitada)
{
	bool _es_proceso_solicitado(t_block_used* bloque){
		return bloque->pid == pid;
	}
	t_block_used* bloqueSolic = list_find(listaBloquesOcupados, (void*) _es_proceso_solicitado);

	if(bloqueSolic != NULL){
		int ptr = bloqueSolic->ptrComienzo + paginaSolicitada * (size_t) __sizePagina__;

		return ptr;
	}
	return NULL;
}

int guardarEnBloque(paginasSolicitadas, pid)
{
/*Aca busco donde garcha meter el proceso*/
	bool _bloque_espacio_suficiente(void *bloque){
		return((t_block_free*)bloque)->cantPag >= paginasSolicitadas;
	}
	t_block_free* bloqueLibre = list_remove_by_condition(listaBloquesLibres, (void*) _bloque_espacio_suficiente);
	if(bloqueLibre != NULL){
		t_block_used* bloqueNuevo = t_block_used_create(pid ,bloqueLibre->ptrComienzo, paginasSolicitadas);
		list_add(listaBloquesOcupados, bloqueNuevo);
		if( bloqueLibre->cantPag > paginasSolicitadas) {
			list_add(listaBloquesLibres, t_block_free_create(bloqueLibre->ptrComienzo + (paginasSolicitadas * __sizePagina__), bloqueLibre->cantPag - paginasSolicitadas ));
		}
		free(bloqueLibre);
		log_info(logger, "Nuevo proceso mProc creado. PID: %d ; Byte Inicial; %p ; Cantidad de Páginas: %d", bloqueNuevo->pid, bloqueNuevo->ptrComienzo ,paginasSolicitadas);
		return 0;
	}
	else{
		Error("Proceso mProc PID: %d rechazado por falta de espacio.", pid);
		return 1;
	}
}

int desfragmentar(){
	int ptrAnt;
	int ptrBloque = 0;
	t_block_used *bloqueAct;
	int i;
	log_info(logger, "Comenzando desfragmentacion...");
	/*if(ptrBloque == NULL){
		ErrorFatal("Error al intentar desfragmentar.");
		return -1;
	}
	else{*/
	int fd = abrirParticionSwap();
	char *swap = mmap((caddr_t) 0, g_Cantidad_Paginas * __sizePagina__ , PROT_READ|PROT_WRITE, MAP_SHARED, fd,  0);
	//char *bloqueNuevo = mmap((caddr_t) 0, getTamanioPagina(ptrBloque) * bloqueAct->cantPag, PROT_WRITE, MAP_SHARED, fd,  getOffset(ptrBloque));
	char *aux = malloc(g_Cantidad_Paginas * __sizePagina__);
	close(fd);
	if (swap == MAP_FAILED) {
			perror("mmap");
			ErrorFatal("Error al desfragmentar (mmap swap)");
	}

	/*if (bloqueNuevo == MAP_FAILED) {
			perror("mmap");
			Error("Error al desfragmentar(mmap bloqueNuevo)");
			return NULL;
	}*/

	if(aux == NULL){
		ErrorFatal("Error al desfragmentar (malloc aux)");
	}

	memcpy(aux, swap, g_Cantidad_Paginas * __sizePagina__);
	memset(swap, '\0', g_Cantidad_Paginas * __sizePagina__);
	if (msync(swap, g_Cantidad_Paginas * __sizePagina__, MS_INVALIDATE)<0){
		ErrorFatal("Error al desfragmentar (msync externo)");
	}
	/*munmap(swap, g_Cantidad_Paginas * __sizePagina__);
	swap = mmap((caddr_t) 0, g_Cantidad_Paginas * __sizePagina__ , PROT_READ|PROT_WRITE, MAP_SHARED, fd,  0);*/

	for(i=0 ; i < listaBloquesOcupados->elements_count; i++){
		bloqueAct = (t_block_used*)list_get(listaBloquesOcupados, i);
		ptrAnt = bloqueAct->ptrComienzo;
		if(ptrBloque != ptrAnt){

			memcpy(swap+ptrBloque, aux+ptrAnt, (size_t) __sizePagina__ * bloqueAct->cantPag);
			bloqueAct->ptrComienzo = ptrBloque;

			/*int ret = msync(bloqueNuevo, __sizePagina__ * bloqueAct->cantPag, MS_INVALIDATE);

			if(ret < 0){
				Error("Error al intentar desfragmentar");
				return -1;
			}

			ret = munmap( swap , getTamanioPagina(ptrAnt) * bloqueAct->cantPag  );
			ret = munmap( bloqueNuevo , getTamanioPagina(ptrBloque) * bloqueAct->cantPag  );
			if (ret < 0){
				ErrorFatal("Error al ejecutar munmap");
			}*/
			list_replace(listaBloquesOcupados, i, bloqueAct);

			if (msync(swap, g_Cantidad_Paginas * __sizePagina__, MS_INVALIDATE)<0){
				ErrorFatal("Error al desfragmentar (msync interno)");
			}

		}
		ptrBloque += bloqueAct->cantPag * __sizePagina__ ;
	}
	if( munmap(swap, g_Cantidad_Paginas * __sizePagina__)<0){
		ErrorFatal("Error al ejecutar munmap");
	}
	free(aux);
	int cantElem = listaBloquesLibres->elements_count;
	for(i= 0; i < cantElem ; i++)
	{
		/*Saco los elementos desde adelante, por eso siempre es 0 :B */
			list_remove_and_destroy_element(listaBloquesLibres, 0, (void*)t_block_free_destroy);
	}
	/*Creo elemento de la lista con las paginas que quedan libres */
	int cantPaginasLibres = g_Cantidad_Paginas - getCantidadPaginasOcupadas();
	if(cantPaginasLibres > 0)
		list_add(listaBloquesLibres, t_block_free_create((int*)ptrBloque, cantPaginasLibres));
	sleep(__retardoCompactacion__);
	log_info(logger, "Gracias por desfragmentar con nosotros :) Vuelvas Prontos");
	return 1;
	//}
}

int existeEspacioContiguo(int paginasSolicitadas){

    bool _bloque_espacio_contiguo(void *bloque) {
        return ((t_block_free*)bloque)->cantPag >= paginasSolicitadas;
    }
    int b = list_any_satisfy(listaBloquesLibres, (void*)_bloque_espacio_contiguo);
    return b;
}

int getCantidadPaginasLibres()
{
	int totalPaginasLibres, i;
	totalPaginasLibres = 0;
	char* _get_cantidad_paginas_libres(t_block_free* bloque) {
		return bloque->cantPag;
	}
	t_list* paginasLibres = list_map(listaBloquesLibres, (void*) _get_cantidad_paginas_libres);

	for(i = 0; i< paginasLibres->elements_count; i++)
	{
		totalPaginasLibres += list_get(paginasLibres, i);
	}

	list_destroy(paginasLibres);

	return totalPaginasLibres;

}


int getCantidadPaginasOcupadas()
{
	int totalPaginasOcupadas, i;
	totalPaginasOcupadas = 0;
	char* _get_cantidad_paginas_ocupadas(t_block_used* bloque) {
		return bloque->cantPag;
	}
	t_list* paginasOcupadas = list_map(listaBloquesOcupados, (void*) _get_cantidad_paginas_ocupadas);

	for(i = 0; i< paginasOcupadas->elements_count; i++)
	{
		totalPaginasOcupadas += list_get(paginasOcupadas, i);
	}

	list_destroy(paginasOcupadas);

	return totalPaginasOcupadas;

}

int crearEstructuraBloquesLibres(){
	listaBloquesLibres = list_create();
	list_add(listaBloquesLibres, t_block_free_create(0, g_Cantidad_Paginas));
	return 1;
}

int crearEstructuraBloques(){
	int x = crearEstructuraBloquesLibres();
	if (x>0)
	x = crearEstructuraBloquesOcupados();
	return x;
}

int crearEstructuraBloquesOcupados(){
	listaBloquesOcupados = list_create();
	return 1;
}


char* getContenido(int ptr)
{

	int fd = abrirParticionSwap();
//	if (fd == NULL)
//	{
//			Error("Error al obtener datos contenidos en la pagina");
//			return NULL;
//	}

	char *datos = mmap((caddr_t) 0, getTamanioPagina(ptr), PROT_READ, MAP_PRIVATE, fd, getOffset(ptr) );

	if (datos == MAP_FAILED) {
			perror("mmap");
			Error("Error al obtener datos contenidos en la pagina");
			return NULL;
	}

	close(fd);
	sleep(__retardoSwap__);
	return datos;
}


size_t getTamanioPagina(int posicion){
	return (size_t) __sizePagina__ + (posicion%4096);
}

off_t getOffset(int posicion){
	return (off_t) (posicion/4096)*4096;
}

int getCorrimiento(int posicion){
	return posicion%4096;
}
