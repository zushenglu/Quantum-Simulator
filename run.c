


#include "Q-sim.h"

#if defined(BENCHMARK_SMALL) || defined(BENCHMARK_LARGE)
#ifndef _OPENMP
#error "Build the benchmark with OpenMP enabled."
#endif

#ifdef BENCHMARK_LARGE
Circuit *build_large_circuit(void);
#else
Circuit *build_benchmark_circuit(int qubits, int rounds);
#endif

int main(void){
#ifdef BENCHMARK_LARGE
    Circuit *circuit = build_large_circuit();
#else
    Circuit *circuit = build_benchmark_circuit(9, 3);
#endif
    enum { PARALLEL_WORKFLOW = 2 };
    int status = workflow_run(PARALLEL_WORKFLOW, circuit);
    if (circuit != NULL){
        FREE_CIRCUIT(circuit);
    }
    return status;
}
#else

int main(void){



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
    int status = workflow_run(0, qc);
    FREE_CIRCUIT(qc);

    return status;
}
#endif
