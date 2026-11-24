#include <stdio.h>
#include <limits.h>
#include <complex.h>
#include <stdlib.h>
#include <math.h>

#include <unistd.h>
#include <pthread.h>

#include <string.h>


#include "qubit.h"
#include "gate.h"

#include "circuit.h"

/* Each qubit owns an ordered linked list of operations. Circuit depth is used
 * to align multi-qubit operations across the qubits they affect. */

/* Print one complex value followed by a space. */
void PRINT_COMPLEX(float complex input){ 
    printf("%.4f%+.4fi ",  creal(input), cimag(input));
    return;
}

void PRINT_MX(float complex **mx, int sl){
    int x = 0;
    int y = 0;

    while (y<sl){
        x = 0;
        while (x<sl){
            PRINT_COMPLEX(mx[y][x]);
            x++;
        }
        y++;
        printf("\n");
    }
}

void PRINT_CIRCUIT_STATE(Circuit* qc, int size){

    for (int i=0;i<size;i++){
        printf("q%d = [%.1f%+.1fi, %.1f%+.1fi]\n", i,   creal(qc->Q[i]->x), cimag(qc->Q[i]->x),
                                                        creal(qc->Q[i]->y), cimag(qc->Q[i]->y));
    }

}

void PRINT_QUBIT_OP(Circuit* qc, int qubit){

    printf("q%d|- ",qubit);
    Operation *op = qc->Q[qubit]->next;
    for (int i=0; i<qc->Q[qubit]->depth; i++){

        if (op->depth != i+1){
            printf("\t\t\t");
            continue;
        }

        if (op->param_ind != -1){
            printf("\t%s ", op->name);
            for (int i=0;i<op->impacted_qbts_num;i++){
                printf("%d-", op->impacted_qbts[i]);
            }
            printf("\t");
        }
        // if single qubit, just print the name
        else{
            if (op->parameters != NULL){
                printf("\t%s( ", op->name);
                int paraNum = op->param_num;
                for (int i=0;i<paraNum;i++){
                    PRINT_COMPLEX(op->parameters[i]);
                }
                printf(")");

            }
            else{
                if (strlen(op->name) < 8){
                    printf("\t%s\t\t", op->name);
                }
                else{
                    printf("\t%s\t", op->name);
                }
            }
        }

        op = op->next;
    }
    printf("\n");
}

void PRINT_OP_INFO(Operation *op){
    if (op == NULL){
        printf("given op is null\n");
        return;
    }

    printf("\nOP info:\n\tname: %s\n\tdepth: %d\n\tnum impacted: %d\n\tindex: ", op->name, op->depth, op->impacted_qbts_num);
    for (int i=0; i<op->impacted_qbts_num;i++){
        printf("%d ", op->impacted_qbts[i]);
    } 
    printf("\n\tparameters :");

    for (int i=0; i<op->param_num;i++){
        PRINT_COMPLEX(op->parameters[i]);
    }
    printf("\n\tparam index: %d",op->param_ind);
    if (op->next == NULL){
        printf("\n\tnext: NULL\n");
    }
    else{
        printf("\n\tnext: %s\n", op->next->name);
    }
}  

void PRINT_CIRCUIT(Circuit* qc, int size){
    for (int i=0;i<size;i++){
        PRINT_QUBIT_OP(qc, i);
    }
}


Circuit* INIT_CIRCUIT(int size){

    Circuit *qc = malloc(sizeof(Circuit));

    /* Initialize the circuit and its qubits. */
    qc->Q = malloc(sizeof(*qc->Q)*size);
    qc->states = NULL;
    qc->depth = 1;

    for (int i=0;i<size;i++){

        Qubit* qubit = INIT_QUBIT(i);
        qc->Q[i] = qubit;
    }
    qc->size = size;

    return qc;
}

static void free_operation(Operation *op){
    if (op->owns_gate_data){
        FREE_GATE(op->gate);
        free(op->parameters);
        if (op->impacted_qbts_num > 1){
            free(op->impacted_qbts);
        }
    }
    free(op);
}

void FREE_CIRCUIT(Circuit *circuit){
    if (circuit == NULL){
        return;
    }
    for (int i = 0; i < circuit->size; i++){
        Qubit *qubit = circuit->Q[i];
        Operation *op = qubit->next;
        while (op != NULL){
            Operation *next = op->next;
            free_operation(op);
            op = next;
        }
        free(qubit);
    }
    free(circuit->Q);
    free(circuit);
}

/* Add a single-qubit operation at the next depth for qbt_ind. */
void Add_OP(Gate* gate, int qbt_ind, Circuit *c, float complex *params, int param_num, char* name){


    Qubit *qubit = c->Q[qbt_ind];
    qubit->depth += 1;
    
    Operation *op = malloc(sizeof(Operation));
    op->depth = qubit->depth;
    op->name = name;
    op->gate = gate;
    op->next = NULL;
    op->impacted_qbts = &qubit->index;
    op->impacted_qbts_num=1;
    op->param_ind=-1;
    op->parameters = params;
    op->param_num = param_num;
    op->owns_gate_data = 0;

    if (qubit->next == NULL){
        qubit->next = op;
        qubit->last = op;
    }
    else{
        qubit->last->next = op;
        qubit->last = op;
    }

    if (qubit->depth > c->depth){
        c->depth = qubit->depth;
    }

}

/* Add a multi-qubit operation at a shared depth across its target qubits. */
void Add_OPM(Gate* gate, int *qbt_ind, int input_num, Circuit *c, float complex *params, int param_num, char* name){

    int max_depth = 0;
    for (int i=0; i<input_num;i++){
        if (c->Q[qbt_ind[i]]->depth > max_depth){
            max_depth = c->Q[qbt_ind[i]]->depth;
        }
    }
    max_depth += 1;

    for (int i=0; i<input_num; i++){
        Operation *op = malloc(sizeof(Operation));
        op->depth = max_depth;
        op->gate = gate;
        op->next = NULL;
        op->parameters = params;
        op->impacted_qbts = qbt_ind;
        op->impacted_qbts_num = input_num;
        op->param_ind = i;
        op->name = name;
        op->param_num = param_num;
        op->owns_gate_data = 0;

        int index = qbt_ind[i];
        Qubit *qubit = c->Q[index];
        
        if (qubit->next == NULL){
            qubit->next = op;
            qubit->last = op;
        }
        else{
            qubit->last->next = op;
            qubit->last = op;
        }

        qubit->depth = max_depth;
        if (qubit->depth > c->depth){
            c->depth = qubit->depth;
        }
    }
}

void PauliX(Circuit *qc, int target_qbt){
    Gate *gate = initPX();
    Add_OP(gate,target_qbt,qc,NULL,0,"X");
    qc->Q[target_qbt]->last->owns_gate_data = 1;
}

void CX(Circuit *qc, int control_qbt, int target_qbt){
    int *qbts = malloc(sizeof(int)*2);
    qbts[0] = control_qbt;
    qbts[1] = target_qbt;
    
    Add_OPM(initCX(), qbts, 2,qc,NULL, 0, "CNOT");
    qc->Q[control_qbt]->last->owns_gate_data = 1;
}

void RZ(Circuit *qc, int target_qbt, float complex rotation){

    float complex *param = malloc(sizeof(float complex));
    *param = rotation;
    Add_OP(RZ_mx(rotation),target_qbt,qc,param, 1, "RZ");
    qc->Q[target_qbt]->last->owns_gate_data = 1;
}

void Hadamard(Circuit *qc, int target_qbt){
    Gate *gate = initH();
    Add_OP(gate,target_qbt,qc,NULL,0,"H");
    qc->Q[target_qbt]->last->owns_gate_data = 1;
}

void Identity(Circuit *qc, int target_qbt){
    Gate *gate=initI();
    Add_OP(gate, target_qbt, qc, NULL, 0, "I");
    qc->Q[target_qbt]->last->owns_gate_data = 1;
}

const Operation *circuit_gate_at(const Circuit *circuit, int wire, int depth){
    if (!circuit || wire < 0 || wire >= circuit->size || depth < 1){
        return NULL;
    }
    const Operation *op = circuit->Q[wire]->next;
    while (op && op->depth < depth){
        op = op->next;
    }
    return op && op->depth == depth ? op : NULL;
}

/* Unlink every endpoint before releasing their shared gate data. */
static void remove_gate(Circuit *circuit, const Operation *selected){
    Operation *removed = NULL;
    for (int i = 0; i < selected->impacted_qbts_num; i++){
        Qubit *wire = circuit->Q[selected->impacted_qbts[i]];
        Operation *previous = NULL;
        Operation **link = &wire->next;
        while ((*link)->depth < selected->depth){
            previous = *link;
            link = &previous->next;
        }
        Operation *op = *link;
        *link = op->next;
        if (op == wire->last){
            wire->last = previous;
            wire->depth = previous ? previous->depth : 0;
        }
        op->next = removed;
        removed = op;
    }
    while (removed){
        Operation *next = removed->next;
        free_operation(removed);
        removed = next;
    }
    /* INIT_CIRCUIT uses one visible column even for an empty circuit. */
    circuit->depth = 1;
    for (int i = 0; i < circuit->size; i++){
        if (circuit->depth < circuit->Q[i]->depth){
            circuit->depth = circuit->Q[i]->depth;
        }
    }
}

const char *circuit_remove_gate(Circuit *circuit, int wire, int depth){
    const Operation *op = circuit_gate_at(circuit, wire, depth);
    if (!op){
        return "No gate at the selected position.";
    }
    remove_gate(circuit, op);
    return NULL;
}

static void insert_at_depth(Circuit *circuit, int index, Operation *op){
    Qubit *wire = circuit->Q[index];
    Operation **link = &wire->next;
    while (*link && (*link)->depth < op->depth){
        link = &(*link)->next;
    }
    op->next = *link;
    *link = op;
    if (!op->next){
        wire->last = op;
        wire->depth = op->depth;
    }
    if (op->depth > circuit->depth){
        circuit->depth = op->depth;
    }
}

static const char *set_gate(Circuit *circuit, CircuitGateEdit edit, const Operation *old){
    int count = edit.kind == CIRCUIT_CX ? 2 : 1;
    int wires[] = {edit.wire, edit.target};
    if (!circuit || edit.depth < 1 || edit.depth == INT_MAX){
        return "Invalid circuit or column.";
    }
    if (edit.kind == CIRCUIT_RZ && !isfinite(edit.angle)){
        return "Enter a finite RZ angle in radians.";
    }
    if (count == 2 && edit.wire == edit.target){
        return "Choose different control and target wires.";
    }
    for (int i = 0; i < count; i++){
        if (wires[i] < 0 || wires[i] >= circuit->size){
            return "Wire is outside the circuit.";
        }
        const Operation *op = circuit_gate_at(circuit, wires[i], edit.depth);
        if (op && !(old && op->depth == old->depth && op->impacted_qbts == old->impacted_qbts)){
            return "A wire is already occupied in this column.";
        }
    }

    /* Build the entire replacement before unlinking the old gate. */
    Operation *op = calloc(1, sizeof(*op));
    if (!op){
        return "Out of memory.";
    }
    op->depth = edit.depth;
    op->param_ind = -1;
    op->owns_gate_data = 1;
    op->impacted_qbts_num = count;
    op->impacted_qbts = &circuit->Q[edit.wire]->index;
    Operation *target = NULL;
    switch (edit.kind){
    case CIRCUIT_H:
        op->name = "H";
        op->gate = initH();
        break;
    case CIRCUIT_X:
        op->name = "X";
        op->gate = initPX();
        break;
    case CIRCUIT_RZ:
        op->name = "RZ";
        op->parameters = malloc(sizeof(*op->parameters));
        if (!op->parameters){
            free_operation(op);
            return "Out of memory.";
        }
        op->parameters[0] = edit.angle;
        op->param_num = 1;
        op->gate = RZ_mx(edit.angle);
        break;
    case CIRCUIT_CX:
        op->name = "CNOT";
        op->param_ind = 0;
        op->impacted_qbts = malloc(sizeof(wires));
        target = malloc(sizeof(*target));
        if (!op->impacted_qbts || !target){
            free(target);
            free_operation(op);
            return "Out of memory.";
        }
        memcpy(op->impacted_qbts, wires, sizeof(wires));
        op->gate = initCX();
        *target = *op;
        target->param_ind = 1;
        target->owns_gate_data = 0;
        break;
    default:
        free_operation(op);
        return "Unknown gate.";
    }
    if (old){
        remove_gate(circuit, old);
    }
    insert_at_depth(circuit, edit.wire, op);
    if (target){
        insert_at_depth(circuit, edit.target, target);
    }
    return NULL;
}

const char *circuit_place_gate(Circuit *circuit, CircuitGateEdit edit){
    return set_gate(circuit, edit, NULL);
}

const char *circuit_update_gate(Circuit *circuit, int wire, int depth, CircuitGateEdit edit){
    const Operation *old = circuit_gate_at(circuit, wire, depth);
    if (!old){
        return "No gate at the selected position.";
    }
    return set_gate(circuit, edit, old);
}
