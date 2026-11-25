#include <stddef.h>
#include "Q-sim.h"

Circuit *build_large_circuit(int size)
{
    Circuit *circuit = INIT_CIRCUIT(size);
    if (circuit == NULL) {
        return NULL;
    }

    /* Put the even-numbered qubits in superposition. */
    Hadamard(circuit, 0);
    Hadamard(circuit, 2);
    Hadamard(circuit, 4);
    Hadamard(circuit, 6);
    Hadamard(circuit, 8);
    Hadamard(circuit, 10);
    Hadamard(circuit, 12);
    Hadamard(circuit, 14);

    /* Entangle each adjacent pair. */
    CX(circuit, 0, 1);
    CX(circuit, 2, 3);
    CX(circuit, 4, 5);
    CX(circuit, 6, 7);
    CX(circuit, 8, 9);
    CX(circuit, 10, 11);
    CX(circuit, 12, 13);
    CX(circuit, 14, 15);

    /* Apply phase rotations in radians. */
    RZ(circuit, 1, 0.25f);
    RZ(circuit, 3, 0.50f);
    RZ(circuit, 5, 0.75f);
    RZ(circuit, 7, 1.00f);
    RZ(circuit, 9, 0.25f);
    RZ(circuit, 11, 0.50f);
    RZ(circuit, 13, 0.75f);
    RZ(circuit, 15, 1.00f);

    /* Connect the pairs in a ring. */
    CX(circuit, 1, 2);
    CX(circuit, 3, 4);
    CX(circuit, 5, 6);
    CX(circuit, 7, 8);
    CX(circuit, 9, 10);
    CX(circuit, 11, 12);
    CX(circuit, 13, 14);
    CX(circuit, 15, 0);

    Hadamard(circuit, 0);
    Hadamard(circuit, 2);
    Hadamard(circuit, 4);
    Hadamard(circuit, 6);
    Hadamard(circuit, 8);
    Hadamard(circuit, 10);
    Hadamard(circuit, 12);
    Hadamard(circuit, 14);

    return circuit;
}


Circuit *build_small_circuit(int size)
{
    Circuit *circuit = INIT_CIRCUIT(size);
    if (circuit == NULL) {
        return NULL;
    }

    /* Put the even-numbered qubits in superposition. */
    Hadamard(circuit, 0);
    Hadamard(circuit, 2);
    Hadamard(circuit, 4);
    Hadamard(circuit, 6);

    /* Entangle each adjacent pair. */
    CX(circuit, 0, 1);
    CX(circuit, 2, 3);
    CX(circuit, 4, 5);
    CX(circuit, 6, 7);

    /* Apply phase rotations in radians. */
    RZ(circuit, 1, 0.25f);
    RZ(circuit, 3, 0.50f);
    RZ(circuit, 5, 0.75f);
    RZ(circuit, 7, 1.00f);

    /* Connect the pairs in a ring. */
    CX(circuit, 1, 2);
    CX(circuit, 3, 4);
    CX(circuit, 5, 6);
    CX(circuit, 7, 0);

    Hadamard(circuit, 0);
    Hadamard(circuit, 2);
    Hadamard(circuit, 4);
    Hadamard(circuit, 6);

    return circuit;
}


int main(int argc, char* argv[]){
    /*
        takes 2 arguments: algo and size
        algo: index of algo. 0 is dense, 2 is paralleled dense, refer to workflow.c 
        size: test circuit size, 8 is small (8 qubits), 16 is large (16 qubits), no other option
    
    */

    int algo = 0; // index based on workflow index
    int size = 8;
    if (argc == 3){
        algo = stoi(argv[1]);
        size = stoi(argv[2]);
    }
    else{
        return 1;
    }
    Circuit *qc;
    if (size == 8){
        qc = build_small_circuit(size);
    }
    else{
        if (size == 16){
            qc = build_large_circuit(size);
        }
    }

    

    PRINT_CIRCUIT(qc,size);
    int status = workflow_run(algo, qc);
    FREE_CIRCUIT(qc);
    return 0;
}
