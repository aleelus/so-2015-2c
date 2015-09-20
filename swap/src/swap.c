/*o
 ============================================================================
 Name        : swap.c
 Author      : SO didn't C that coming
 Version     : 1.0
 Copyright   : SO didn't C that coming - UTN FRBA 2015
 Description : Trabajo Practivo Sistemas Operativos 2C 2015
 Testing	 :
 ============================================================================
 */

#include "swap.h"
#include <stdlib.h>
#define __TEST_FRAGMENTACION__ 0

 g_Ejecutando = 1;
 cantHilos=0;

int main(void) {
	//Si el tercer parametro es true graba en archivo y muestra en pantalla sino solo graba en archivo
	logger = log_create(NOMBRE_ARCHIVO_LOG, "swap", false, LOG_LEVEL_TRACE);

	// Levantamos el archivo de configuracion.
	LevantarConfig();


		if(!crearParticionSwap())
			ErrorFatal("Error al crear la particion de swap");
		else{
			if(__TEST_FRAGMENTACION__){
				if( crearEstructuraBloques() > 0){
				crearEntornoParaTestDesfragmentacion();
				ejecutarOrden(1, "311225112");
				}
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
	FILE *ptr;
	getComienzoParticionSwap(&ptr);
	if(ptr!=NULL){
		list_remove(listaBloquesLibres, 0);
		list_add(listaBloquesLibres, t_block_free_create((int*)ptr, 1));
		list_add(listaBloquesLibres, t_block_free_create((int*)ptr + 2*__sizePagina__, 1));
		list_add(listaBloquesOcupados, t_block_used_create(24, (int*)ptr + __sizePagina__, 1));
		list_add(listaBloquesOcupados, t_block_used_create(45, (int*)ptr + 3*__sizePagina__,3));
	}
	else
		Error("Error al crear test para desfragmentacion");
}

int crearParticionSwap(){
			int res;
			char* cmd = string_new();
			string_append(&cmd, "dd if=/dev/zero of=/home/utnso/git/");
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
		char* pid = DigitosNombreArchivo(buffer,&posActual);
		char* paginasSolicitadas = DigitosNombreArchivo(buffer, &posActual);
		int res = crearProceso(atoi(pid), atoi(paginasSolicitadas));
		EnviarRespuesta(CREA_PROCESO, (res < 0) ? __FALLO__ : guardarEnBloque(paginasSolicitadas), NULL);
		free(pid);
		free(paginasSolicitadas);

	}
		break;
	case SOLICITA_MARCO:
	{
		int posActual = 2;
		char* pid = DigitosNombreArchivo(buffer,&posActual);
		char* paginaSolicitada = DigitosNombreArchivo(buffer, &posActual);
		EnviarRespuesta(CREA_PROCESO, atoi(paginaSolicitada), pid);
		free(pid);
		free(paginaSolicitada);
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
	free(buffer);
}

int finalizarProceso(char* buffer){
	int posActual = 2;
	char* pid = DigitosNombreArchivo(buffer,&posActual);
	long PID = atol(pid);
	free(pid);

	bool _es_bloque_a_reemplazar(t_block_used* bloque)
		{
			return(bloque->pid == pid);
		}
		//Obtengo el bloque que contiene el marco a reemplazar.
		t_block_used *bloque = list_remove_by_condition(listaBloquesOcupados, (void*) _es_bloque_a_reemplazar );

		if (bloque != NULL){
		//Obtengo la direccion del marco
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
				Error("Error al finalizar proceso %d: no fue posible abrir la particion de swap", PID);
				return -1;
			}
			//Verifico el estado del archivo que estoy abriendo
			if (stat(dir, &sbuf) == -1) {
				free(dir);
				Error("Error al finalizar proceso %d: Verificar estado de archivo particion de swap", PID);
				return -1;
			}
			free(dir);

			getComienzoParticionSwap(&ptrComienzoParticion);
			char *datos = mmap((caddr_t) 0, (size_t) __sizePagina__ * bloque->cantPag, PROT_WRITE, MAP_SHARED, fd, ptrComienzoParticion - bloque->ptrComienzo );
			close(fd);
			memset(datos, '\0', __sizePagina__);

			int ret = msync(datos, __sizePagina__, MS_INVALIDATE);
			if(ret < 0){
				Error("Error al intentar sincronizar datos de pagina %d", PID);
				return -1;
			}
			ret = munmap( datos , __sizePagina__ );
			if (ret < 0){
				ErrorFatal("Error al ejecutar munmap");
			}
			return 1;
		}
		else{
			Error("Error al intentar finalizar proceso %d: No se encontro el bloque correspondiente al mismo", PID);
			return -1;
		}

}


int reemplazarMarco(char* buffer){
	int posActual = 2;
	char* pid = DigitosNombreArchivo(buffer,&posActual);
	char* pagina = DigitosNombreArchivo(buffer, &posActual);
	int pag = atoi(pagina);
	long PID = atol(pid);
	free (pagina);
	free(pid);

	bool _es_bloque_a_reemplazar(t_block_used* bloque)
	{
		return(bloque->pid == pid);
	}
	//Obtengo el bloque que contiene el marco a reemplazar.
	t_block_used *bloque = list_remove_by_condition(listaBloquesOcupados, (void*) _es_bloque_a_reemplazar );

	if (bloque != NULL){
	//Obtengo la direccion del marco
		FILE* ptrComienzoParticion;
		int fd;
		FILE* ptrMarco = bloque->ptrComienzo + (pag * __sizePagina__);
		char * dir = string_new();
		string_append(&dir, "/home/utnso/git/");
		string_append(&dir, g_Nombre_Swap);
		string_append(&dir, ".bin");
		ptrComienzoParticion = fopen(dir, "r");
		struct stat sbuf;

		if ((fd = open(dir, O_RDONLY)) == -1) {
			Error("Error al reemplazar marco: no fue posible abrir la particion de swap");
			return -1;
		}
		//Verifico el estado del archivo que estoy abriendo
		if (stat(dir, &sbuf) == -1) {
			Error("Error al reemplazar marco: Verificar estado de archivo particion de swap");
			return -1;
		}
		free(dir);

		getComienzoParticionSwap(&ptrComienzoParticion);
		char *datos = mmap((caddr_t) 0, (size_t) __sizePagina__, PROT_WRITE, MAP_SHARED, fd, ptrComienzoParticion - ptrMarco );
		close(fd);
		memcpy(datos, buffer[2], __sizePagina__);
		int ret = msync(datos, __sizePagina__, MS_INVALIDATE);
		if(ret < 0){
			Error("Error al intentar sincronizar datos de pagina %d", PID);
			return -1;
		}
		ret = munmap( datos , __sizePagina__ );
		if (ret < 0){
			ErrorFatal("Error al ejecutar munmap");
		}
		return 1;
	}
	else{
		Error("No se encontro ningun bloque correspondiente al pid %d", PID);
		return -1;
	}
}




int crearProceso(int pid, int paginasSolicitadas){
	int totalPaginasLibres = getCantidadPaginasLibres();
	if(paginasSolicitadas > totalPaginasLibres)
		return -1;
	else
	{
		if(!existeEspacioContiguo(paginasSolicitadas))
			desfragmentar();

		return 1;
	}
}

FILE* getPtrPaginaProcesoSolic(pid, paginaSolicitada)
{
	bool _es_proceso_solicitado(t_block_used* bloque){
		return (bloque->pid);
	}
	t_block_used* bloqueSolic = list_find(listaBloquesOcupados, (void*) _es_proceso_solicitado);

	if(bloqueSolic != NULL){
		FILE* ptr = bloqueSolic->ptrComienzo + paginaSolicitada * (size_t) __sizePagina__;

		return ptr;
	}
		return NULL;
}

int guardarEnBloque(paginasSolicitadas)
{
/*Aca busco donde garcha meter el proceso*/
		bool _bloque_espacio_suficiente(void *bloque){
			return((t_block_free*)bloque)->cantPag >= paginasSolicitadas;
		}
		t_block_free* bloqueLibre = list_remove_by_condition(listaBloquesLibres, (void*) _bloque_espacio_suficiente);
		if(bloqueLibre != NULL){
			t_block_free_create(bloqueLibre->ptrComienzo, paginasSolicitadas);
			free(bloqueLibre);
			return 0;
		}
		else
			return 1;
}

int desfragmentar(){
	FILE* ptrBloque, *ptrAnt;
	t_block_used *bloqueAct;
	int i = 0;
	getComienzoParticionSwap(&ptrBloque);
	if(ptrBloque == NULL){
		ErrorFatal("Error al intentar desfragmentar.");
		return -1;
	}
	else{

		for(i ; i < listaBloquesOcupados->elements_count; i++){
			bloqueAct = (t_block_used*)list_get(listaBloquesOcupados, i);
			ptrAnt = bloqueAct->ptrComienzo;
			if(ptrBloque != ptrAnt){
				memcpy(ptrBloque, ptrAnt, (size_t) __sizePagina__);
				bloqueAct->ptrComienzo = ptrBloque;
				list_replace(listaBloquesOcupados, i, bloqueAct);
			}
			ptrBloque = ptrAnt + (size_t) __sizePagina__ ;
		}
		int cantElem = listaBloquesLibres->elements_count;
		for(i= 0; i < cantElem ; i++)
		{
			/*Saco los elementos desde adelante, por eso siempre es 0 :B */
				list_remove_and_destroy_element(listaBloquesLibres, 0, (void*)t_block_free_destroy);
		}
		/*Creo elemento de la lista con las paginas que quedan libres */
		int cantPaginasLibres = getCantidadPaginasOcupadas();
		cantPaginasLibres =  g_Cantidad_Paginas - cantPaginasLibres;
		if(cantPaginasLibres > 0)
			list_add(listaBloquesLibres, t_block_free_create((int*)ptrBloque, cantPaginasLibres));

		return 1;
	}
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

void getComienzoParticionSwap(FILE** ptr){
	char * dir = string_new();
		string_append(&dir, "/home/utnso/git/");
		string_append(&dir, g_Nombre_Swap);
		string_append(&dir, ".bin");
		*ptr = fopen(dir, "r");
		int v = fclose(*ptr);
		if (v != 0){
			Error("Error al cerrar particion de swap.");
		}
}

int crearEstructuraBloquesLibres(){
	FILE* ptr;
	getComienzoParticionSwap(&ptr);
	if(ptr == NULL){
		ErrorFatal("Error al crear la estructura de nodos libres.");
		return -1;
	}

	listaBloquesLibres = list_create();
	list_add(listaBloquesLibres, t_block_free_create((int*)ptr, g_Cantidad_Paginas));
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


char* getContenido(FILE* ptr)
{
	char* dir = string_new();
	int fd;
	string_append(&dir, "/home/utnso/git/");
	string_append(&dir, g_Nombre_Swap);
	string_append(&dir, ".bin");
	struct stat sbuf;

	if ((fd = open(dir, O_RDONLY)) == -1) {
		perror("open");
		exit(1);
	}
	//Verifico el estado del archivo que estoy abriendo
	if (stat(dir, &sbuf) == -1) {
		perror("stat");
		exit(1);
	}
	FILE *ptrComienzoParticion;
	getComienzoParticionSwap(&ptrComienzoParticion);
	char *datos = mmap((caddr_t) 0, (size_t) __sizePagina__, PROT_READ, MAP_PRIVATE, fd, ptrComienzoParticion - ptr );

	if (datos == MAP_FAILED) {
			perror("mmap");
			Error("Error al obtener datos contenidos en la pagina");
			return NULL;
	}
	close(fd);
	return datos;
}
