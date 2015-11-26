#include "particionSwap.h"

t_block_used *t_block_used_create(long pid, int ptrComienzo, int cantPag) {
    t_block_used* new = malloc(sizeof(t_block_used));
    new->pid = pid;
    new->ptrComienzo = ptrComienzo;
    new->cantPag = cantPag;
    new->reads = 0;
    new->writes = 0;
    return new;
}

void t_block_used_destroy(t_block_used *self) {
    free(self);
}

t_block_free *t_block_free_create(int ptrComienzo, int cantPag) {
    t_block_free* new = malloc(sizeof(t_block_free));
    new->ptrComienzo = ptrComienzo;
    new->cantPag = cantPag;
    return new;
}

void t_block_free_destroy(t_block_free *self) {
    free(self);
}
