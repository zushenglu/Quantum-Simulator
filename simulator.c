#include <stdio.h>
#include <complex.h>

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


void simulate(Qubit *qc, int target_qbt){




}

void Transform(float complex * vector, int vec_dim, float complex ** mx, int mx_dim){

    for (int i=0;i<mx_dim;i++){
        float complex temp = vector[i];
        vector[i] = 0;
        for (int j=0;j<mx_dim;j++){
            vector[i] += mx[i][j] * temp;
        }
    }

}




