
#include <stdio.h>
#include <complex.h>
#include <math.h>

#include "qubit.h"
#include "gate.h"
#include "circuit.h"

int main(int argnum, char** arg){

    double complex x = 4.0 * I; 

    printf("z = %.1f%+.1fi\n", creal(x), cimag(x));
    
    // ex with qubit
    Qubit qubit;
    // INIT_QBT(qubit);
    qubit.x = 4.0*I + 5;
    qubit.y = 2.0*I + 3;

    printf("mx = %.1f%+.1fi\n", creal(qubit.x), cimag(qubit.x));
    printf("my = %.1f%+.1fi\n", creal(qubit.y), cimag(qubit.y));

    int SIZE = 5;

    Circuit *qc = INIT_CIRCUIT(SIZE);

    // add parameterized single gate
    RZ(qc,2,M_PI);

    // add regular single gate
    PauliX(qc,0);

    // // add unparameterized multi-gate
    CX(qc,0,1);
    CX(qc,1,0);
    CX(qc,4,3);

    RZ(qc,4,M_PI/4);
    CX(qc,2,3);
    CX(qc,1,3);
    CX(qc,0,4);
    


    PRINT_CIRCUIT(qc,SIZE);


    return 0;
}