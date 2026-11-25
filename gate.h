#ifndef Qgate
#define Qgate

#include <complex.h>

typedef struct struct_gate {
    int dimension;
    int param_id;
    float complex **mx;
    int control_count; // default is 0 as single qubit, multi-qubit gate must have it
} Gate; 

void PRINT_GATE(Gate *gate,int dimension);
void FREE_GATE(Gate *gate);

Gate* RZ_mx(float input);

Gate* initCX();

Gate* initPX();

Gate* initH();

Gate* initI();

#endif
