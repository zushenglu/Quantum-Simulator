#include <stdio.h>
#include <complex.h>
#include <stdlib.h>

#include "circuit.h"

typedef struct struct_gate {
    int dimension;
    int param_id;
    float complex **mx;
} Gate;

typedef struct struct_operation {
    char * name;
    float complex *parameters;
    int param_ind;
    Gate *gate;
    struct struct_operation *next;
} Operation;

typedef struct struct_qubit {
    int index;
    int depth;
    float complex x;
    float complex y;
    Operation *next;
    Operation *last;
} Qubit; 

typedef struct struct_circuit {
    Qubit **Q;
    int depth;
} Circuit;



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

        if (op->param_ind != -1){
            printf("\t%s %d\t", op->name, op->param_ind);
        }
        else{
            printf("\t%s\t", op->name);
        }

        op = op->next;
    }
    printf("\n");
}

// speace instead of new line
void PRINT_COMPLEX(float complex input){ 
    printf("%.1f%+.1fi ",  creal(input), cimag(input));
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

Qubit* INIT_QUBIT(int index){

    Qubit *qubit = malloc(sizeof(Qubit));
    qubit->depth = 0;
    qubit->index = index;
    qubit->x = 1;
    qubit->y = 0;
    qubit->next = NULL;
    qubit->last = NULL;

    return qubit;
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

    return qc;
}

// add a operation, gate is a pointer to gate, qbt_ind is qubit index,
// circuit is circuit, param is a list of parameters.
// only works for single qubit gate
void Add_OP(Gate* gate, int qbt_ind, Circuit *c, float complex *param, char* name){

    Qubit *qubit = c->Q[qbt_ind];
    qubit->depth += 1;
    
    Operation *op = malloc(sizeof(Operation));
    op->name = name;
    op->gate = gate;
    op->next = NULL;
    op->param_ind=-1;
    op->parameters = param;

    if (qubit->next == NULL){
        qubit->next = op;
        qubit->last = op;
    }
    else{
        qubit->last->next = op;
        qubit->last = op;
    }

}

void Add_OPM(Gate* gate, int *qbt_ind, int input_num, Circuit *c, float complex *param, char* name){

    for (int i=0; i<input_num; i++){
        Operation *op = malloc(sizeof(Operation));
        op->gate = gate;
        op->next = NULL;
        op->parameters = param;
        op->param_ind = i;
        op->name = name;

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
        qubit->depth+=1;
    }
}

int main(int argnum, char** arg){

    double complex x = 4.0 * I; 

    printf("z = %.1f%+.1fi\n", creal(x), cimag(x));
    
    // ex with qubit
    Qubit qubit;
    // INIT_QBT(qubit);
    qubit.x = 4.0*I + 5;
    qubit.y = 2.0*I + 3;

    printf("mx = %.1f%+.1fi\n", creal(qubit.x), cimag(qubit.x));
    printf("my = %.1f%+.1fi\n", creal(qubit.y), cimag(qubit.y));

    int SIZE = 3;

    Circuit *qc = INIT_CIRCUIT(SIZE);

    PRINT_CIRCUIT_STATE(qc,SIZE);

    Gate PauliX;
    PauliX.dimension=2;
    PauliX.param_id=NULL;
    PauliX.mx = malloc(sizeof(float complex)*PauliX.dimension);
    for (int i=0;i<PauliX.dimension;i++){
        float complex *row = malloc(sizeof(float complex)*PauliX.dimension);
        PauliX.mx[i] = row;
    }
    PauliX.mx[0][0] = 0;
    PauliX.mx[0][1] = 1;
    PauliX.mx[1][0] = 1;
    PauliX.mx[1][1] = 0;

    PRINT_MX(PauliX.mx, PauliX.dimension);

    Gate CNOT;
    CNOT.dimension=4;
    CNOT.mx = malloc(sizeof(float complex)*CNOT.dimension);
    for (int i=0;i<CNOT.dimension;i++){
        float complex *row = malloc(sizeof(float complex)*CNOT.dimension);
        CNOT.mx[i] = row;
    }
    CNOT.mx[0][0] = 1;
    CNOT.mx[0][1] = 0;
    CNOT.mx[0][2] = 0;
    CNOT.mx[0][3] = 0;

    CNOT.mx[1][0] = 0;
    CNOT.mx[1][1] = 1;
    CNOT.mx[1][2] = 0;
    CNOT.mx[1][3] = 0;

    CNOT.mx[2][0] = 0;
    CNOT.mx[2][1] = 0;
    CNOT.mx[2][2] = 0;
    CNOT.mx[2][3] = 1;
    
    CNOT.mx[3][0] = 0;
    CNOT.mx[3][1] = 0;
    CNOT.mx[3][2] = 1;
    CNOT.mx[3][3] = 0;

    Add_OP(&PauliX,0,qc,NULL,"PauliX");
    Add_OP(&PauliX,1,qc,NULL,"PauliX");
    int a[2] = {0,1};
    Add_OPM(&CNOT, a, 2, qc, NULL, "CNOT");

    int b[2] = {1,0};
    Add_OPM(&CNOT, b, 2, qc, NULL, "CNOT");

    PRINT_QUBIT_OP(qc, 0);
    PRINT_QUBIT_OP(qc, 1);


    // Gate PauliY;
    // PauliY.dimension=2;
    // PauliY.param_id=0;
    // PauliY.mx = malloc(sizeof(float complex)*PauliY.dimension*PauliY.dimension);
    // PauliY.mx[0][0] = 0;
    // PauliY.mx[0][1] = -I;
    // PauliY.mx[1][0] = I;
    // PauliY.mx[1][1] = 0;

    // Gate PauliZ;
    // PauliZ.dimension=2;
    // PauliZ.param_id=0;
    // PauliZ.mx = malloc(sizeof(float complex)*PauliZ.dimension*PauliZ.dimension);
    // PauliZ.mx[0][0] = 1;
    // PauliZ.mx[0][1] = 0;
    // PauliZ.mx[1][0] = 0;
    // PauliZ.mx[1][1] = -1;

    return 0;
}