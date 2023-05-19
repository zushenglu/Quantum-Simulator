#ifndef Qgate
#define Qgate

#include <complex.h>

typedef struct struct_gate {
    int dimension;
    int param_id;
    float complex **mx;
} Gate; 

void PRINT_GATE(Gate *gate,int dimension);

Gate* RZ_mx(float input);

Gate* initCX();

Gate* initPX();

#endif