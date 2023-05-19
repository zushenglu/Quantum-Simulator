#ifndef Qcircuit
#define Qcircuit

#include <complex.h>

struct struct_qubit;
struct struct_gate;

typedef struct struct_circuit {
    struct struct_qubit **Q;
    int depth;
} Circuit;

void PRINT_COMPLEX(float complex input);

void PRINT_MX(float complex **mx, int sl);
void PRINT_CIRCUIT_STATE(Circuit* qc, int size);

void PRINT_QUBIT_OP(Circuit* qc, int qubit);
void PRINT_CIRCUIT(Circuit* qc, int size);

Circuit* INIT_CIRCUIT(int size);
void Add_OP(struct struct_gate * gate, int qbt_ind, Circuit *c, float complex *params, int param_num, char* name);
void Add_OPM(struct struct_gate * gate, int *qbt_ind, int input_num, Circuit *c, float complex *params, int param_num, char* name);

void PauliX(Circuit *qc, int target_qbt);
void CX(Circuit *qc, int control_qbt, int target_qbt);
void RZ(Circuit *qc, int target_qbt, float complex rotation);


#endif