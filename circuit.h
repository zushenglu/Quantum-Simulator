#ifndef Qcircuit
#define Qcircuit

#include <complex.h>
#include "gate.h"

struct struct_qubit;
struct struct_gate;

typedef struct struct_operation {
    int depth;
    char * name;
    float complex *parameters;
    int param_num;           // Number of gate parameters.
    int param_ind;           // This qubit's position in a multi-qubit operation.
    int *impacted_qbts;      // Indices of qubits affected by the operation.
    int impacted_qbts_num;   // Number of affected qubits.
    int single_qbt_index;    // Stable storage for a single-qubit operation.
    Gate *gate;
    struct struct_operation *next;
} Operation;

typedef struct struct_circuit {
    struct struct_qubit **Q;
    float complex* states;
    int depth;
    int size;
} Circuit;

void PRINT_COMPLEX(float complex input);

void PRINT_MX(float complex **mx, int sl);
void PRINT_CIRCUIT_STATE(Circuit* qc, int size);
void PRINT_OP_INFO(Operation* op);
void PRINT_QUBIT_OP(Circuit* qc, int qubit);
void PRINT_CIRCUIT(Circuit* qc, int size);

Circuit* INIT_CIRCUIT(int size);
/* Releases all circuit-owned qubits, operations, parameters, and gates. */
void FREE_CIRCUIT(Circuit *circuit);
/* The circuit takes ownership of gate and params after they are added. */
void Add_OP(struct struct_gate * gate, int qbt_ind, Circuit *c, float complex *params, int param_num, char* name);
/* Also takes ownership of qbt_ind; all affected operations share it and gate. */
void Add_OPM(struct struct_gate * gate, int *qbt_ind, int input_num, Circuit *c, float complex *params, int param_num, char* name);

void PauliX(Circuit *qc, int target_qbt);
void CX(Circuit *qc, int control_qbt, int target_qbt);
void RZ(Circuit *qc, int target_qbt, float complex rotation);
void Hadamard(Circuit *qc, int target_qbt);


#endif
