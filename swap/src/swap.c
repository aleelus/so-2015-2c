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
		else
			crearEstructuras();
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




void Comenzar_Consola() {

	int corte_consola = -1;
	printf("Consola para ingresar Comandos al planificador\n");
	while (corte_consola != 0) {
		corte_consola = operaciones_consola();
	}
	printf("Se termino la ejecucion de la consola del filesystem\n");
}

int operaciones_consola() {

	int variable_seleccion;
	scanf("%d", &variable_seleccion);
	printf("Comando incorrecto.\n");
	return 0;
}


int ejecutarOrden(orden){
	switch(orden){
	case CREA_PROCESO:
		break;
	case SOLICITA_MARCO:
		break;
	case REEMPLAZA_MARCO:
		break;
	case FINALIZAR_PROCESO:
		break;
	}
}

int crearEstructuras(){

}
