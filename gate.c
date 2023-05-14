#include <stdio.h>
#include <complex.h>
#include <stdlib.h>

#include "gate.h"

// speace instead of new line
void PRINT_COMPLEX(float complex input){ 
    printf("%.1f%+.1fi ",  creal(input), cimag(input));
    return;
}

void PRINT_GATE(Gate *gate, int dimension){

    printf("\n");
    for (int i=0;i<dimension;i++){

        for (int j=0;j<dimension;j++){
            printf("%.1f%+.1fi ",  creal(gate->mx[i][j]), cimag(gate->mx[i][j]));
        }
        printf("\n");
    }
    printf("\n");

}


/*
// not sure
Gate* TO_UNITARY(Gate * gate, int dimension){
    // a matrix is unitary if a matrix multiplication with its transposed conjugate == I, AKA Hermition
    

    return gate;
}

// not sure
Gate* CREATE_GATE(float complex **mx, int dimension){

    Gate *temp = malloc(sizeof(Gate));
    temp->dimension=dimension;
    temp->mx = mx;

    temp = TO_UNITARY(temp, dimension);

    return temp; 
}

*/
