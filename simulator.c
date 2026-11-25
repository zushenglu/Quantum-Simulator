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

static float complex **tensor_product_parallel(float complex *const *matrix, int size,
                                      float complex *const *factor){

    float complex **product = malloc(2 * size * sizeof(*product));
    for (int i=0; i<2*size;i++){
        product[i] = malloc(2 * size * sizeof(*product[i]));
    }

    #pragma omp parallel for collapse(2) default(none) shared(product, matrix, size, factor)
    for (int row = 0; row < 2 * size; row++){
        for (int col = 0; col < 2 * size; col++){
            product[row][col] = matrix[row / 2][col / 2] * factor[row % 2][col % 2];
        }
    }
    return product;
}


// directly calculate TP from an array of gate matrix
void **tensor_product_direct(int dimension,
    float complex matrix[dimension][dimension],
    Gate** gates, int gates_len){

    /*
        trick told by chatgpt, it cited this paper, i did not give it a read
            https://www.mdpi.com/2225-1146/9/2/18, 
        
        just trying it out and see if it works. 
    */
    for (int r = 0; r < dimension; r++) {
        for (int c = 0; c < dimension; c++) {

            float complex value = 1.0f;
            int stride = dimension;

            for (int k = 0; k < gates_len; k++) {
                int dims = gates[k]->dimension;
                stride /= dims;

                int ri = (r / stride) % dims;
                int ci = (c / stride) % dims;

                value *= gates[k]->mx[ri][ci];
            }

            matrix[r][c] = value;
        }
    }   
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

static float complex **build_gate_matrix_parallel(int qubits, float complex *const *identity,
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

        float complex **product = tensor_product_parallel(matrix, size, factor);
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

static float complex **build_controlled_matrix_parallel(const Operation *op, int qubits,
                                               int size, float complex *const *identity){
    int control = op->impacted_qbts[0];
    int target = op->impacted_qbts[1];
    float complex zero_data[2][2] = {{1, 0}, {0, 0}};
    float complex one_data[2][2] = {{0, 0}, {0, 1}};
    float complex *zero[] = {zero_data[0], zero_data[1]};
    float complex *one[] = {one_data[0], one_data[1]};
    /* Borrow the controlled gate's lower-right block for the control=1 term. */
    float complex *target_gate[] = {op->gate->mx[2] + 2, op->gate->mx[3] + 2};

    float complex **matrix = build_gate_matrix_parallel(qubits, identity, control, zero, -1, NULL);
    float complex **active = build_gate_matrix_parallel(qubits, identity, control, one, target, target_gate);
    
    #pragma omp parallel for collapse(2) default(none) shared(matrix, active, size)
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

void simulate_parallel(Circuit* circuit){
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

            float complex *next_state = MX_MAP_parallel(statevector, size, matrix, size);
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

static void generate_depth_p_mapping_(const Operation *const *cursor, Circuit* circuit, int depth, int* p_ind){
    /*
        generate permutation pattiong for current depth based on the circuit
            depth = current depth count

        populate p_ind with permutation matting 
            p_ind should have same size as circuit->size
    */

    int qubits = circuit->size;

    // inititalize p_ind
    for (int i=0;i<qubits;i++){
        p_ind[i] = i;
    } 

    int procced_ind[qubits];
    for (int i=0;i<qubits;i++)
        procced_ind[i]=0;
    
    int next_p = 0;

    // iterate the depth to build p map
    for (int i = 0; i < qubits; i++){

        const Operation *op = cursor[i];
        if (op == NULL || op->depth != depth){
            // for non-multi-qubit op, we don't care the it's position
            // we permute for multi-qubit and use whichever location it ends with
            continue;
        }
        if (procced_ind[i] == 1)
            continue;
        if (op->impacted_qbts_num > 1){
            // create swap mapping for multi-qbt
            int *impacted_qbts = op->impacted_qbts;
            int gate_ind = 0;
            while (gate_ind < op->impacted_qbts_num){            
                if (impacted_qbts[gate_ind] != p_ind[next_p]){
                    // find 1 in vec
                    int qbt_ind_ind = next_p;
                    while (qbt_ind_ind < qubits){
                        // search for target index to swap;
                        if (p_ind[qbt_ind_ind] == impacted_qbts[gate_ind])
                            break;
                        qbt_ind_ind++;
                    }
                    // swap
                    int temp =p_ind[next_p] ;
                    p_ind[next_p] = p_ind[qbt_ind_ind];
                    p_ind[qbt_ind_ind]= temp;
                }

                procced_ind[impacted_qbts[gate_ind]]=1;
                gate_ind ++;
                next_p ++;
            }

        }
    }
}

static int permute_index(int index, int *p_ind, int n) {
    // permute index, calculate new basis-state vector index
    int result = 0;

    for (int k = 0; k < n; k++) {
        int bit = (index >> p_ind[k]) & 1;
        result |= bit << (n - 1 - k);
    }

    return result;
}

void simulate2(Circuit* circuit){
    /*
        optimize based on simulate from gate to depth based matrix generation
        runtime: O(g*2^(2n)) to O(d*2^(2n)) where 
            g = # of gates
            n = # of qubits
            d = circuit depth

        
        generally, we can replace identity to the gate on corresponding index during the TP
        for multi-qubite gates, we can swap indexs so multi-qubit gates impacts continuous
        qubits. (permutation equivalent property, detail refer to Kronecker) The permutation
        matrix can be further simplized to an index map.
            > A tp B = P mp (B tp A) mp P_dagger
    */
    int qubits = circuit->size;
    int size = pow(2, qubits);
    float complex *statevector = calloc(size, sizeof(*statevector));
    statevector[0] = 1;

    float complex (*matrix)[size] = malloc((size_t)size * sizeof(*matrix));

    Gate *identity = initI();
    const Operation **cursor = malloc(qubits * sizeof(*cursor));
    for (int i = 0; i < qubits; i++){
        cursor[i] = circuit->Q[i]->next;
    }

    int *p_ind = malloc(sizeof(int)*qubits);
    Gate **gates = malloc(sizeof(*gates)*qubits);
    int *pass_ind = calloc(qubits, sizeof(int));

    for (int depth = 1; depth <= circuit->depth; depth++){

        // reset pass_ind
        for (int k=0;k<qubits;k++)
            pass_ind[k]=0;

        int next_ind = 0;
        generate_depth_p_mapping_(cursor, circuit, depth, p_ind);

        
        // extract depth gate vectors
        for (int i = 0; i < qubits; i++){

            int op_ind = p_ind[i];

            if (pass_ind[op_ind] == 1){
                continue;
            }

            const Operation *op = cursor[op_ind];
            // float complex **matrix;
            if (op == NULL || op->depth != depth){
                pass_ind[op_ind] = 1;
                gates[next_ind++] = identity;
                continue;
            }

            if (op->impacted_qbts_num == 1){
                pass_ind[op_ind] = 1;
                gates[next_ind++] = op->gate;
                cursor[op_ind] = op->next;
                continue;
            }

            if (op->impacted_qbts_num > 1){
                // matrix = build_controlled_matrix(op, qubits, size, identity->mx);
                gates[next_ind++] = op->gate;
                for (int j = 0; j < op->impacted_qbts_num; j++){
                    int index = op->impacted_qbts[j];
                    cursor[index] = cursor[index]->next;
                    pass_ind[index]=1;
                }
            }
        }        

        tensor_product_direct(size, matrix, gates, next_ind); // perform direct tp, now we have U'

        // create maping based off p
        int *mapping = malloc(size * sizeof(int));
        for (int i = 0; i < size; i++)
            mapping[i] = permute_index(i, p_ind, qubits);
        
        // Reorder statevector, temp = P|phi>
        float complex *temp = malloc(size * sizeof(*temp));

        for (int i = 0; i < size; i++)
            temp[mapping[i]] = statevector[i];

        // apply U', next = U'P|phi>
        float complex *next = MX_MAP2(temp, size, size, matrix);

        // restore ordering with P^dagger, next_state = P^dagggerU'P|phi> = U|phi>
        float complex *next_state = malloc(size * sizeof(*next_state));

        for (int i = 0; i < size; i++)
            next_state[i] = next[mapping[i]];

        free(statevector);
        free(mapping);
        free(next);
        free(temp);
        statevector=next_state;
    }
    free(pass_ind);
    free(gates);
    free(matrix);
    free(p_ind);
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

float complex* MX_MAP_parallel(float complex * vector, int vec_dim, float complex ** mx, int mx_dim){

    float complex *new = (float complex*) calloc(vec_dim, sizeof(float complex));

    #pragma omp parallel for default(none) shared(new, mx, mx_dim, vector)
    for (int i=0;i<mx_dim;i++){

        for (int j=0;j<mx_dim;j++){
            new[i] += mx[i][j] * vector[j];
        }
    }

    return new;

}

float complex* MX_MAP2(float complex * vector, int vec_dim, int dimension,
    float complex mx[dimension][dimension]){

    float complex *new = (float complex*) calloc(vec_dim, sizeof(float complex));

    for (int i=0;i<dimension;i++){

        for (int j=0;j<dimension;j++){
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

