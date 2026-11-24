#ifndef Qcircuit
#define Qcircuit

#include <complex.h>

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
    struct struct_gate *gate;
    struct struct_operation *next;
    int owns_gate_data;
} Operation;

typedef struct struct_circuit {
    struct struct_qubit **Q;
    float complex* states;
    int depth;
    int size;
} Circuit;

typedef enum { CIRCUIT_H, CIRCUIT_X, CIRCUIT_RZ, CIRCUIT_CX } CircuitGateKind;
typedef struct {
    CircuitGateKind kind;
    int wire;     /* Zero-based wire, or control for CX. */
    int depth;    /* One-based column. */
    int target;   /* CX only. */
    float angle;  /* RZ only, in radians. */
} CircuitGateEdit;

const Operation *circuit_gate_at(const Circuit *circuit, int wire, int depth);
/* Edits return NULL on success or a static error message. */
const char *circuit_place_gate(Circuit *circuit, CircuitGateEdit edit);
const char *circuit_remove_gate(Circuit *circuit, int wire, int depth);
const char *circuit_update_gate(Circuit *circuit, int wire, int depth, CircuitGateEdit edit);

void PRINT_COMPLEX(float complex input);

void PRINT_MX(float complex **mx, int sl);
void PRINT_CIRCUIT_STATE(Circuit* qc, int size);
void PRINT_OP_INFO(Operation* op);
void PRINT_QUBIT_OP(Circuit* qc, int qubit);
void PRINT_CIRCUIT(Circuit* qc, int size);

Circuit* INIT_CIRCUIT(int size);
void FREE_CIRCUIT(Circuit *circuit);
/* These insertion functions borrow gate, params, name, and any qubit array. */
void Add_OP(struct struct_gate * gate, int qbt_ind, Circuit *c, float complex *params, int param_num, char* name);
void Add_OPM(struct struct_gate * gate, int *qbt_ind, int input_num, Circuit *c, float complex *params, int param_num, char* name);

void PauliX(Circuit *qc, int target_qbt);
void CX(Circuit *qc, int control_qbt, int target_qbt);
void RZ(Circuit *qc, int target_qbt, float complex rotation);
void Hadamard(Circuit *qc, int target_qbt);


#endif
