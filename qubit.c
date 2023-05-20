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

// void PRINT_COMPLEX(float complex input){ 
//     printf("%.1f%+.1fi ",  creal(input), cimag(input));
//     return;
// }

void PRINT_QUBIT_STAT(Qubit* qubit){
    
    printf("qubit info:\n  name:  q%d \n  index: %d\n  depth: %d\n",
            qubit->index, qubit->index, qubit->depth);
    printf("  state: [ ");
    printf("%.1f%+.1fi ",  creal(qubit->x), cimag(qubit->x));
    printf(", ");
    printf("%.1f%+.1fi ]\n",  creal(qubit->y), cimag(qubit->y));
}

