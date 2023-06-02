#include "gate.h"

#include <stdio.h>
#include <complex.h>
#include <stdlib.h>
#include <math.h>

#include "circuit.h" // for rz debug only, remove once fixed

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

Gate* RZ_mx(float input){
    Gate *g = malloc(sizeof(Gate));
    g->dimension = 2;
    g->param_id=-1;
    float complex **mx = malloc(sizeof(float complex*)*2);
    for (int i=0;i<2;i++){
        float complex *row = malloc(sizeof(float complex)* 2);
        mx[i] = row;
    }


    mx[0][0] = cexp(-I/2*input);
    // printf()
    mx[0][1] = 0;
    mx[1][0] = 0;
    mx[1][1] = cexp(I/2*input);

    g->mx = mx;
    // PRINT_MX(mx,2);

    return g;
}

Gate* initCX(){
    Gate *CNOT = malloc(sizeof(Gate));
    CNOT->dimension=4;
    CNOT->mx = malloc(sizeof(float complex)*CNOT->dimension);
    for (int i=0;i<CNOT->dimension;i++){
        float complex *row = malloc(sizeof(float complex)*CNOT->dimension);
        CNOT->mx[i] = row;
    }
    CNOT->mx[0][0] = 1;
    CNOT->mx[0][1] = 0;
    CNOT->mx[0][2] = 0;
    CNOT->mx[0][3] = 0;

    CNOT->mx[1][0] = 0;
    CNOT->mx[1][1] = 1;
    CNOT->mx[1][2] = 0;
    CNOT->mx[1][3] = 0;

    CNOT->mx[2][0] = 0;
    CNOT->mx[2][1] = 0;
    CNOT->mx[2][2] = 0;
    CNOT->mx[2][3] = 1;
    
    CNOT->mx[3][0] = 0;
    CNOT->mx[3][1] = 0;
    CNOT->mx[3][2] = 1;
    CNOT->mx[3][3] = 0;

    return CNOT;
}

Gate* initPX(){
    Gate *PauliX = malloc(sizeof(Gate));
    PauliX->dimension=2;
    PauliX->param_id=NULL;
    PauliX->mx = malloc(sizeof(float complex)*PauliX->dimension);
    for (int i=0;i<PauliX->dimension;i++){
        float complex *row = malloc(sizeof(float complex)*PauliX->dimension);
        PauliX->mx[i] = row;
    }
    PauliX->mx[0][0] = 0;
    PauliX->mx[0][1] = 1;
    PauliX->mx[1][0] = 1;
    PauliX->mx[1][1] = 0;
    return PauliX;
}

Gate* initH(){
    Gate *Hada = malloc(sizeof(Gate));
    Hada->dimension=2;
    Hada->param_id=NULL;
    Hada->mx = malloc(sizeof(float complex)*Hada->dimension);
    for (int i=0;i<Hada->dimension;i++){
        float complex *row = malloc(sizeof(float complex)*Hada->dimension);
        Hada->mx[i] = row;
    }
    float complex num = 1/sqrt(2);
    Hada->mx[0][0] = num;
    Hada->mx[0][1] = num;
    Hada->mx[1][0] = num;
    Hada->mx[1][1] = -num;
    return Hada;
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

Gate* initI(){
    Gate *IdentGate = malloc(sizeof(Gate));
    IdentGate->dimension = 2;
    IdentGate->param_id = NULL;
    float complex ** Ident = malloc(sizeof(float complex*) * 2);
    Ident[0] = malloc(sizeof(float complex) * 2);
    Ident[1] = malloc(sizeof(float complex) * 2);
    Ident[0][0] = 1;
    Ident[1][1] = 1;
    Ident[0][1] = 0;
    Ident[1][0] = 0;
    IdentGate->mx = Ident;
    return IdentGate;
}