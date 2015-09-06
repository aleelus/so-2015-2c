/*
 * particionSwap.h
 *
 *  Created on: 5/9/2015
 *      Author: utnso
 */

#ifndef PARTICIONSWAP_H_
#define PARTICIONSWAP_H_

typedef struct t_block_used;
typedef struct t_block_free;

typedef struct t_block_used
{
	long pid;
	int ptrComienzo;
	int cantPag;
	struct t_block_used* ptrNext;
}t_block_used;

typedef struct t_block_free
{
	int ptrComienzo; //Comienzo del espacio libre
	int cantPag;
	struct t_block_free* ptrNext;
}t_block_free;



#endif /* PARTICIONSWAP_H_ */
