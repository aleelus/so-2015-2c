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

 g_Ejecutando = 1;
 cantHilos=0;

int main(void) {
	//Si el tercer parametro es true graba en archivo y muestra en pantalla sino solo graba en archivo
	logger = log_create(NOMBRE_ARCHIVO_LOG, "swap", false, LOG_LEVEL_TRACE);

	// Levantamos el archivo de configuracion.
	LevantarConfig();

	if(!ENTREGA1)
	{
		if(!crearParticionSwap())
			ErrorFatal("Error al crear la particion de swap");
		else{
			if( crearEstructuraBloques() > 0){
			crearEntornoParaTestDesfragmentacion();
			ejecutarOrden(1, "311225112");
			}
		}
	}
	if(1 == 0){
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
	}
	return 0;
}

int crearEntornoParaTestDesfragmentacion(){
	FILE *ptr;
	getComienzoParticionSwap(&ptr);
	if(ptr!=NULL){
		list_remove(listaBloquesLibres, 0);
		list_add(listaBloquesLibres, t_block_free_create((int*)ptr, 1));
		list_add(listaBloquesLibres, t_block_free_create((int*)ptr + 2*g_Tamanio_Pagina, 1));
		list_add(listaBloquesOcupados, t_block_used_create(24, (int*)ptr + g_Tamanio_Pagina, 1));
		list_add(listaBloquesOcupados, t_block_used_create(45, (int*)ptr + 3*g_Tamanio_Pagina,1));
	}
	else
		Error("Error al crear test para desfragmentacion");
	return 0;
}

int crearParticionSwap(){
			int res;
			char* cmd = string_new();
			string_append(&cmd, "dd if=/dev/zero of=/home/utnso/git/");
			string_append(&cmd, g_Nombre_Swap);
			string_append(&cmd, ".bin bs=");
			string_append(&cmd, string_itoa(g_Tamanio_Pagina));
			string_append(&cmd, " count=");
			string_append(&cmd, string_itoa(g_Cantidad_Paginas));
			if((res = system(cmd))== -1)
			{
				perror("Error al intentar crear particion de swap");
				return -1;
			}
			return 1;
}

void procesarBuffer(char* buffer, long unsigned tamanioBuffer){
	FILE * archivo = fopen("texto.txt","w");
	fwrite(buffer,1,tamanioBuffer,archivo);
	fclose(archivo);
}

void enviarArchivo(){
	FILE * archivo = fopen("texto2.txt","r");
	char * contenido;
	fseek(archivo,0L,SEEK_END);
	long unsigned tamanioA = ftell(archivo);

	contenido = malloc(tamanioA+1);
	memset(contenido,0,tamanioA+1);
	rewind(archivo);
	fread(contenido,1,tamanioA,archivo);

	EnviarDatos(socket_Memoria,contenido,tamanioA, YO);

	free(contenido);
	fclose(archivo);
}


int ejecutarOrden(int orden, char* buffer){
	switch(orden){
	case CREA_PROCESO:
	{
		int posActual = 2;
		char* pid = DigitosNombreArchivo(buffer,&posActual);
		//posActual = posActual +1;
		char* paginasSolicitadas = DigitosNombreArchivo(buffer, &posActual);
		crearProceso(atoi(pid), atoi(paginasSolicitadas));
	}
		break;
	case SOLICITA_MARCO:
		break;
	case REEMPLAZA_MARCO:
		break;
	case FINALIZAR_PROCESO:
		break;
	}
}


int crearProceso(int pid, int paginasSolicitadas){
	/*1.Verificar cantidad de paginas libres si son suficientes
	 *2. Verificar si hay espacio contiguo para cantidad de paginas necesarias  */
	int totalPaginasLibres = getCantidadPaginasLibres();
	if(paginasSolicitadas > totalPaginasLibres)
	{
		/*Aca deberia devolver al admin de memoria que no se pudo crear el proceso*/
	}
	else
	{
		if(existeEspacioContiguo(paginasSolicitadas))
		{
			/*Aca busco donde garcha meter el proceso*/
			bool _bloque_espacio_suficiente(void *bloque){
				return((t_block_free*)bloque)->cantPag >= paginasSolicitadas;
			}
			t_block_free* bloqueLibre = list_remove_by_condition(listaBloquesLibres, (void*) _bloque_espacio_suficiente);
			if(bloqueLibre != NULL){
				t_block_free_create(bloqueLibre->ptrComienzo, paginasSolicitadas);
				free(bloqueLibre);
			/*Enviar respusta de ejecucion ok*/
				char* rspOk;
				string_append(&rspOk,"41");
				string_append(&rspOk, INIT_OK);
				EnviarDatos(socket_Memoria, rspOk, strlen(rspOk), COD_ADM_SWAP);
				free(rspOk);
			}
		else{
			char* rspFail;
			string_append(&rspFail,"41");
			string_append(&rspFail, INIT_FAIL);
			EnviarDatos(socket_Memoria, rspFail, strlen(rspFail), COD_ADM_SWAP);
			free(rspFail);
			}
		}
		else
		{
			desfragmentar();
		}

	}
	return 1;
};


int desfragmentar(){
	FILE* ptrBloque, *ptrAnt;
	t_block_used *bloqueAct;
	int i = 0;
	getComienzoParticionSwap(&ptrBloque);
	if(ptrBloque == NULL){
		Error("Error al intentar desfragmentar");
		return -1;
	}
	else{

		for(i ; i < listaBloquesOcupados->elements_count; i++){
			bloqueAct = (t_block_used*)list_get(listaBloquesOcupados, i);
			ptrAnt = bloqueAct->ptrComienzo;
			if(ptrBloque != ptrAnt){
				memcpy(ptrBloque, ptrAnt, (size_t) g_Tamanio_Pagina);
				bloqueAct->ptrComienzo = ptrBloque;
				list_replace(listaBloquesOcupados, i, bloqueAct);
			}
			ptrBloque = ptrAnt + (size_t) g_Tamanio_Pagina ;

		}

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

};

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
