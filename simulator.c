#include <stdio.h>
#include <complex.h>
#include <stdlib.h>
#include <math.h>

#include "simulator.h"
#include "qubit.h"
#include "gate.h"
#include "circuit.h"
#include "convertor.h"


/* The simulator builds full-system gate matrices from tensor products and
 * applies them to a complex state vector one circuit depth at a time. */

void PRINT_VECTOR(float complex *vector, int size){

    for (int i=0;i<size;i++){
        printf("\n%d\t", i);

        PRINT_COMPLEX(vector[i]);
    }
    printf("\n");

}

static void free_matrix(float complex **matrix, int size){
    for (int row = 0; row < size; row++){
        free(matrix[row]);
    }
    free(matrix);
}

/* Append one 2x2 factor to a square tensor product. */
static float complex **tensor_product(float complex *const *matrix, int size,
                                      float complex *const *factor){
    float complex **product = malloc(2 * size * sizeof(*product));
    for (int row = 0; row < 2 * size; row++){
        product[row] = malloc(2 * size * sizeof(*product[row]));
        for (int col = 0; col < 2 * size; col++){
            product[row][col] = matrix[row / 2][col / 2] * factor[row % 2][col % 2];
        }
    }
    return product;
}

/* Qubit 0 is the low bit, so append its factor last. */
static float complex **build_gate_matrix(int qubits, float complex *const *identity,
                                         int first_index, float complex *const *first,
                                         int second_index, float complex *const *second){
    float complex unit = 1;
    float complex *unit_row = &unit;
    float complex **matrix = &unit_row;
    int size = 1;

    for (int index = qubits - 1; index >= 0; index--){
        float complex *const *factor = identity;
        if (index == first_index){
            factor = first;
        }
        else if (index == second_index){
            factor = second;
        }

        float complex **product = tensor_product(matrix, size, factor);
        if (size > 1){
            free_matrix(matrix, size);
        }
        matrix = product;
        size *= 2;
    }
    return matrix;
}

static float complex **build_controlled_matrix(const Operation *op, int qubits,
                                               int size, float complex *const *identity){
    int control = op->impacted_qbts[0];
    int target = op->impacted_qbts[1];
    float complex zero_data[2][2] = {{1, 0}, {0, 0}};
    float complex one_data[2][2] = {{0, 0}, {0, 1}};
    float complex *zero[] = {zero_data[0], zero_data[1]};
    float complex *one[] = {one_data[0], one_data[1]};
    /* Borrow the controlled gate's lower-right block for the control=1 term. */
    float complex *target_gate[] = {op->gate->mx[2] + 2, op->gate->mx[3] + 2};

    float complex **matrix = build_gate_matrix(qubits, identity, control, zero, -1, NULL);
    float complex **active = build_gate_matrix(qubits, identity, control, one, target, target_gate);
    for (int row = 0; row < size; row++){
        for (int col = 0; col < size; col++){
            matrix[row][col] += active[row][col];
        }
    }
    free_matrix(active, size);
    return matrix;
}

typedef struct struct_gate_queue{
};

/* Return the operations scheduled at a given circuit depth. */
Operation** OP_BY_DEPTH(Circuit* circuit, int depth){

    int size = circuit->size;
    Operation** op_arr = malloc(sizeof(Operation*)*size);

    Operation *op;
    Qubit *qbt;
    for (int i=0;i<size;i++){  
        qbt = circuit->Q[i];
        op = qbt->next;

        if (op == NULL){
            op_arr[i] = NULL;
            continue;
        }

        if (op->depth != depth){
            op_arr[i] = NULL;
            continue;
        }

        op_arr[i] = op;
    }

    return op_arr;


}

void simulate2(Circuit* circuit){

    int tot_qbt = circuit->size;
    int sv_size = pow(2,tot_qbt);
    // printf("%ld\n", sv_size );
    // printf("%zu\n", sizeof(float complex));
    // printf("%zu\n", SIZE_MAX);
    float complex *statevector = calloc(sv_size, sizeof(float complex));

    float complex **Identity = initI()->mx;

    /* Initialize the state vector to |0...0>. */
    statevector[0] = 1;


    int circuit_depth = circuit->depth;
    Operation **op_arr;

    // printf("circuit depth: %d\n", circuit_depth);

    // iterate by depth
    for (int d=1; d<=circuit_depth; d++){
        int i=0; 

        // op_arr = GATE_BY_DEPTH(circuit, d);

        

    }


    ResUnit **result = to_prob(statevector,sv_size);
    PRINT_RESULT(result,sv_size);


}

void simulate(Circuit* circuit){
    int qubits = circuit->size;
    int size = pow(2, qubits);
    float complex *statevector = calloc(size, sizeof(*statevector));
    statevector[0] = 1;

    Gate *identity = initI();
    const Operation **cursor = malloc(qubits * sizeof(*cursor));
    for (int i = 0; i < qubits; i++){
        cursor[i] = circuit->Q[i]->next;
    }

    for (int depth = 1; depth <= circuit->depth; depth++){
        for (int i = 0; i < qubits; i++){
            const Operation *op = cursor[i];
            if (op == NULL || op->depth != depth){
                continue;
            }

            float complex **matrix;
            if (op->impacted_qbts_num > 1){
                matrix = build_controlled_matrix(op, qubits, size, identity->mx);
                for (int j = 0; j < op->impacted_qbts_num; j++){
                    int index = op->impacted_qbts[j];
                    cursor[index] = cursor[index]->next;
                }
            }
            else{
                matrix = build_gate_matrix(qubits, identity->mx, i, op->gate->mx, -1, NULL);
                cursor[i] = op->next;
            }

            float complex *next_state = MX_MAP(statevector, size, matrix, size);
            free_matrix(matrix, size);
            free(statevector);
            statevector = next_state;
        }
    }

    free(cursor);
    FREE_GATE(identity);
    ResUnit **result = to_prob(statevector, size);
    PRINT_RESULT(result, size);
    FREE_RESULT(result, size);
    free(statevector);
}

void process_qbt(Circuit*, int qbt_ind){
    
}


float complex* MX_MAP(float complex * vector, int vec_dim, float complex ** mx, int mx_dim){

    float complex *new = (float complex*) calloc(vec_dim, sizeof(float complex));

    for (int i=0;i<mx_dim;i++){

        for (int j=0;j<mx_dim;j++){
            new[i] += mx[i][j] * vector[j];
        }
    }

    return new;

}

float complex* TS_PD(float complex *vector1, int vec1_dim, float complex *vector2, int vec2_dim){

    float complex* new = (float complex*) calloc(vec1_dim * vec2_dim, sizeof(float complex));

    for (int i=0; i<vec1_dim;i++){

        for (int j=0; i<vec2_dim; j++){

            new[i*vec2_dim+vec2_dim] = vector1[i] * vector2[j];

        }
    }

    return new;

}

void Transform(int * qbts, Circuit *qc, int qbt_num){

    

}

float complex* Init_QS(int qbts){
    float complex* a = calloc(2*qbts, sizeof(float complex));
    return a;
}

