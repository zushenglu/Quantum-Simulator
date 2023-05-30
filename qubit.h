#ifndef Qbit
#define Qbit

#include <complex.h>
struct struct_operation;

typedef struct struct_qubit {
    int index;
    int depth;
    float complex x;
    float complex y;
    struct struct_operation *next;
    struct struct_operation *last;
} Qubit; 

Qubit* INIT_QUBIT(int index);

void PRINT_QUBIT_STAT(Qubit* qubit);

#endif