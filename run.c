
#include <stdio.h>
#include <complex.h>
#include <math.h>
#include <stdlib.h>

#include "qubit.h"
#include "gate.h"
#include "circuit.h"
#include "simulator.h"
// brk, sbrk

int main(int argnum, char** arg){

    double complex x = 4.0 * I; 
    char *str = "abc";
    printf("z = %.1f%+.1fi\n", creal(x), cimag(x));
    
    // ex with qubit
    Qubit qubit;
    // INIT_QBT(qubit);
    qubit.x = 4.0*I + 5;
    qubit.y = 2.0*I + 3;

    printf("mx = %.1f%+.1fi\n", creal(qubit.x), cimag(qubit.x));
    printf("my = %.1f%+.1fi\n", creal(qubit.y), cimag(qubit.y));

    int SIZE = 3;

    Circuit *qc = INIT_CIRCUIT(SIZE);
 
    // add parameterized single gate
    // RZ(qc,2,M_PI);

    // add regular single gate
    // PauliX(qc,0);
    Hadamard(qc,0);

    // // add unparameterized multi-gate
    // CX(qc,0,1);
    CX(qc,0,1);

    RZ(qc,2,M_PI/4);
    RZ(qc,1,M_PI/4);
    RZ(qc,1,M_PI/4);


    printf("hi there \n", qc->Q[2]->next);

    PRINT_MX(qc->Q[2]->next->gate->mx,qc->Q[2]->next->gate->dimension);

    // RZ(qc,2,M_PI/4);

    // PauliX(qc,3);
    PRINT_CIRCUIT(qc,SIZE);

    // PRINT_QUBIT_STAT(qc->Q[3]);

    // float complex *arr = malloc(sizeof(float complex)*2);
    // Qubit* qbt = qc->Q[3];
    // arr[0] = qbt->x;
    // arr[1] = qbt->y;

    // Operation* op = qbt->next;
    // Gate* tg = op->gate;

    // arr = MX_MAP(arr,2,tg->mx, tg->dimension);
    // qbt->x = arr[0];
    // qbt->y = arr[1];

    // PRINT_QUBIT_STAT(qc->Q[3]);

    
    simulate(qc);


    // PRINT_MX(qc->Q[3]->next->gate->mx, qc->Q[3]->next->gate->dimension);

    return 0;
}