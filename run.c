
#include <stdio.h>
#include <complex.h>
#include <math.h>
#include <stdlib.h>

// #include "qubit.h"
// #include "gate.h"
// #include "circuit.h"
// #include "simulator.h"
#include "Q-sim.h"
// brk, sbrk

int main(int argnum, char** arg){



    int SIZE = 3;

    Circuit *qc = INIT_CIRCUIT(SIZE);
    Hadamard(qc,0);
    CX(qc,0,1);
    RZ(qc,2,M_PI/4);
    RZ(qc,1,M_PI/4);
    PauliX(qc,2);
    CX(qc,2,0);
    CX(qc,1,2);
    Hadamard(qc,1);



    PRINT_CIRCUIT(qc,SIZE);
    simulate(qc);

    return 0;
}