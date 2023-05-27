#include <stdio.h>
#include <complex.h>
#include <stdlib.h>

#include "simulator.h"
#include "qubit.h"
#include "gate.h"
#include "circuit.h"


/*
    general idea:
        read the circuit by circuit depth
        apply gate to each qubit
        pass resulting circuit onto measurement 

    how to deal with multi-qubit gate:
        since circuit have placeholder gates, all qubits affected by the
        gate will be in the same depth. 
        
        In this case we check the param_id and take tensor product
            should be a vector of [2xn] x 1 where n = num of qubits

        then map the vector with given gate to get final state vector V

        (need more research on the topic and proof it)
        resulting vector should give amplitude for each qubit in the form
        (assuming i start on 1):
            param_qubit[i].a = V[(n/(2i))-1] 
            param_qubit[i].b = V[(n/(2i))] 

        
    
    check this site:
        https://learn.microsoft.com/en-us/azure/quantum/concepts-multiple-qubits


*/

/*
    Damn ngl i think its kinda fucked lmao ...

    path1: read them all together, create a big array for all states and modifi them by steps
        requires restructre the simulator
        requires some math for details of implementaion

    path2: keep the same structure, but when entanglement happens, make it dependent while keep
    the original state. modify it when state ended
        requires extra process for entanglement and keeping the state
        requires some tha for details

    path3: Stop acting like you know the way like you know the rules! 
        there are no rules man we are lost :(  
    

*/

void simulate(Circuit* circuit, int target_qbt){



}

void process_qbt(Circuit*, int qbt_ind){
    
}

float complex* MX_MAP(float complex * vector, int vec_dim, float complex ** mx, int mx_dim){

    float complex *new = (float complex*) calloc(vec_dim, sizeof(float complex));

    for (int i=0;i<mx_dim;i++){

        for (int j=0;j<mx_dim;j++){
            // PRINT_COMPLEX(vector[j]);
            // printf(" * ");
            // PRINT_COMPLEX(mx[i][j]);
            // printf("\n");
            new[i] += mx[i][j] * vector[j];
        }
    }

    // printf("new state: ");
    // PRINT_COMPLEX(vector[0]);
    // PRINT_COMPLEX(vector[1]);
    // printf("\n");

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





/*

a = [1/sqrt2 1/sqrt2] b = [1/sqrt2 1/sqrt2]
a ox b = [ 1/2 1/2 1/2 1/2]

[1,0,0,0     [1/2    [1/2   [00           [1/4
 0,1,0,0  ox  1/2  =  1/2 =  01   => pr =  1/4
 0,0,0,1      1/2     1/2    10            1/4
 0,0,1,0]     1/2]    1/2    11]           1/4]

*/

