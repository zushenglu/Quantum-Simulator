#include <stdio.h>
#include <complex.h>
#include <stdlib.h>

#include "qubit.h"

Qubit* INIT_QUBIT(int index){

    Qubit *qubit = malloc(sizeof(Qubit));
    qubit->depth = 0;
    qubit->index = index;
    qubit->x = 1;
    qubit->y = 0;
    qubit->next = NULL;
    qubit->last = NULL;

    return qubit;
}


