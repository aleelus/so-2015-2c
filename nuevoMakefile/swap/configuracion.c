/*
 * configuracion.c
 *
 *  Created on: 5/9/2015
 *      Author: utnso
 */
#include "configuracion.h"

#if 1 // METODOS CONFIGURACION //
void LevantarConfig() {
	t_config* config = config_create(PATH_CONFIG);
	// Nos fijamos si el archivo de conf. pudo ser leido y si tiene los parametros
	if (config->properties->table_current_size != 0) {

		// Puerto de escucha
		if (config_has_property(config, "PUERTO_ESCUCHA")) {
			g_Puerto = config_get_int_value(config, "PUERTO_ESCUCHA");
		} else
			Error("No se pudo leer el parametro PUERTO_ESCUCHA");
		if (config_has_property(config, "NOMBRE_SWAP")) {
			g_Nombre_Swap = config_get_string_value(config, "NOMBRE_SWAP");
		} else
			Error("No se pudo leer el parametro NOMBRE_SWAP");
		if (config_has_property(config, "CANTIDAD_PAGINAS")) {
			g_Cantidad_Paginas = config_get_int_value(config, "CANTIDAD_PAGINAS");
		} else
			Error("No se pudo leer el parametro CANTIDAD_PAGINAS");
		if (config_has_property(config, "TAMANIO_PAGINA")) {
			__sizePagina__ = config_get_int_value(config, "TAMANIO_PAGINA");
		} else
			Error("No se pudo leer el parametro TAMANIO_PAGINA");
		if (config_has_property(config, "RETARDO_COMPACTACION")) {
			__retardoCompactacion__ = config_get_int_value(config, "RETARDO_COMPACTACION");
		} else
		Error("No se pudo leer el parametro RETARDO_COMPACTACION");
		if (config_has_property(config, "RETARDO_SWAP")) {
					__retardoSwap__ = config_get_int_value(config, "RETARDO_SWAP");
				} else
				Error("No se pudo leer el parametro RETARDO_COMPACTACION");
	} else {
		ErrorFatal("No se pudo abrir el archivo de configuracion");
	}
	if (config != NULL ) {
		free(config);
	}
}

#endif
