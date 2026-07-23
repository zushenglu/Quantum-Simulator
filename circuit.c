#include <stdio.h>
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
    qc->Q = malloc(sizeof(Qubit)*size);
    qc->depth = 1;

    for (int i=0;i<size;i++){

        Qubit* qubit = INIT_QUBIT(i);
        qc->Q[i] = qubit;
    }
    qc->size = size;

    return qc;
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
    op->impacted_qbts = &qbt_ind;
    op->impacted_qbts_num=1;
    op->param_ind=-1;
    op->parameters = params;
    op->param_num = param_num;

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
}

void CX(Circuit *qc, int control_qbt, int target_qbt){
    int *qbts = malloc(sizeof(int)*2);
    qbts[0] = control_qbt;
    qbts[1] = target_qbt;
    
    Add_OPM(initCX(), qbts, 2,qc,NULL, 0, "CNOT");
}

void RZ(Circuit *qc, int target_qbt, float complex rotation){

    float complex *param = malloc(sizeof(float complex));
    *param = rotation;
    Add_OP(RZ_mx(rotation),target_qbt,qc,param, 1, "RZ");
}

void Hadamard(Circuit *qc, int target_qbt){
    Gate *gate = initH();
    Add_OP(gate,target_qbt,qc,NULL,0,"H");
}

void Identity(Circuit *qc, int target_qbt){
    Gate *gate=initI();
    Add_OP(gate, target_qbt, qc, NULL, 0, "I");
}