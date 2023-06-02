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

// #define INIT_QBT(X) Qubit X = {.x = 1, .y =0}

/*
    General idea:
        input numQubit a constant;

        circuit consist many qubits, stored in hashmap with chainning
        Hashtable contains qubits on each index, which is also the head of Linked List
        Qubit consists amplitude for basis |0> and |1>

        auxiliary: temporary allocated qubit that serves as tempory memory
    
    creating circuit:
        user state number of qubits in circuit
        user add gates one at a time

    issue:
        for multi-qubit gates, how do we deal with them?

    idea for issue:
        we store one extra information in the qubit, call it depth

        when applying a muti-qubit gate, we take the largest depth

        for all qubits involved that have depth < largest depth,
            add placeholder gates until equal depth
        
        add multi-qubit gate to them 

*/

/*
    design for circuit;
    each qubit connects to a linkedlist of operations instead of gates;
    
    operation consists parameters and gate

    add function will add parameters and additional information for the gates
        this is the non static part of the gate
    
    add function will connect operation with the gate so it can be reused
        gate is not static, even parameterized gate will be dealt by operation
    
    

*/

// static gate:


// speace instead of new line
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
            // printf("\tI");
            printf("\t\t\t");
            continue;
        }

        // if not single qubit, print order
        if (op->param_ind != -1){
            printf("\t%s ", op->name);
            for (int i=0;i<op->impacted_qbts_num;i++){
                printf("%d-", op->impacted_qbts[i]);
            }
            printf("\t");
        }
        // if single qubit, just print the name
        else{
            // if parameterized, print parameter
            if (op->parameters != NULL){
                printf("\t%s( ", op->name);

                // printf("paranum: %d\n", op->param_num);
                // PRINT_COMPLEX(op->parameters[0]);

                int paraNum = op->param_num;
                for (int i=0;i<paraNum;i++){
                    PRINT_COMPLEX(op->parameters[i]);
                }
                printf(")\t");

            }
            // if simple gate without parameter, simply print the thing
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

void PRINT_CIRCUIT(Circuit* qc, int size){
    for (int i=0;i<size;i++){
        PRINT_QUBIT_OP(qc, i);
    }
}


Circuit* INIT_CIRCUIT(int size){

    Circuit *qc = malloc(sizeof(Circuit));

    // init circuit
    qc->Q = malloc(sizeof(Qubit)*size);
    qc->depth = 1;

    for (int i=0;i<size;i++){

        Qubit* qubit = INIT_QUBIT(i);
        qc->Q[i] = qubit;
    }
    qc->size = size;

    return qc;
}

// add a operation, gate is a pointer to gate, qbt_ind is qubit index,
// circuit is circuit, param is a list of parameters.
// only works for single qubit gate
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

// for adding multiqubit gates, parameterized or not
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

    // printf("rz input: ");
    // PRINT_COMPLEX(rotation);
    // printf("\n");
    float complex *param = malloc(sizeof(float complex));
    *param = rotation;
    // PRINT_COMPLEX(*param);
    // printf("\n");
    // PRINT_MX(RZ_mx(rotation)->mx,2);

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