/*
 * particionSwap.h
 *
 *  Created on: 5/9/2015
 *      Author: utnso
 */

#ifndef PARTICIONSWAP_H_
#define PARTICIONSWAP_H_
#include <stdio.h>

typedef struct t_block_used;
typedef struct t_block_free;

typedef struct t_block_used
{
	long pid;
	FILE* ptrComienzo;
	int cantPag;
	struct t_block_used* ptrNext;
}t_block_used;

typedef struct t_block_free
{
	FILE* ptrComienzo; //Comienzo del espacio libre
	int cantPag;
	struct t_block_free* ptrNext;
}t_block_free;


#endif /* PARTICIONSWAP_H_ */

t_block_used *t_block_used_create(long pid, int* ptrComienzo, int cantPag);
void t_block_used_destroy(t_block_used *self) ;
void t_block_free_destroy(t_block_free *self) ;
t_block_free *t_block_free_create(int* ptrComienzo, int cantPag) ;
